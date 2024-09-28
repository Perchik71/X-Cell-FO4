// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <detours/Detours.h>
#include <xc_patch.h>
#include <xc_plugin.h>

namespace xc
{
	bool patch::start(const char* section)
	{
		if (g_plugin->get_settings()->read_bool(section, get_name(), false))
			return start_impl();
		return false;
	}

	bool patch::patch_mem_nop(uintptr_t offset, size_t size) const noexcept
	{
		auto pvTarget = reinterpret_cast<uint8_t*>(offset);

		DWORD dwOld = 0;
		if (!VirtualProtect(pvTarget, size, PAGE_EXECUTE_READWRITE, &dwOld))
			return false;

		for (size_t i = 0; i < size; i++)
			pvTarget[i] = 0x90;

		// Ignore if this fails, the memory was copied either way
		VirtualProtect(pvTarget, size, dwOld, &dwOld);
		return true;
	}

	bool patch::patch_mem(uintptr_t offset, uint8_t* buffer, size_t size) const noexcept
	{
		auto pvTarget = reinterpret_cast<void*>(offset);
		auto pvMemory = reinterpret_cast<void*>(buffer);

		DWORD dwOld = 0;
		if (!VirtualProtect(pvTarget, size, PAGE_EXECUTE_READWRITE, &dwOld))
			return false;

		memcpy(pvTarget, pvMemory, size);

		// Ignore if this fails, the memory was copied either way
		VirtualProtect(pvTarget, size, dwOld, &dwOld);
		return true;
	}

	bool patch::patch_mem(uintptr_t offset, initializer_list<uint8_t> list) const noexcept
	{
		auto pvTarget = reinterpret_cast<void*>(offset);
		auto uSize = list.size();

		DWORD dwOld = 0;
		if (!VirtualProtect(pvTarget, uSize, PAGE_EXECUTE_READWRITE, &dwOld))
			return false;

		memcpy(pvTarget, (void*)(&(*list.begin())), uSize);

		// Ignore if this fails, the memory was copied either way
		VirtualProtect(pvTarget, uSize, dwOld, &dwOld);
		return true;
	}

	uintptr_t patch::patch_iat(HMODULE current_module, const char* import_module_name,
		const char* func_name, uintptr_t func) const noexcept
	{
		return Detours::IATHook((uintptr_t)current_module, import_module_name, func_name, func);
	}

	uintptr_t patch::patch_iat_delay(HMODULE current_module, const char* import_module_name,
		const char* func_name, uintptr_t func) const noexcept
	{
		return Detours::IATDelayedHook((uintptr_t)current_module, import_module_name, func_name, func);
	}

	uintptr_t patch::patch_vtable_func(uintptr_t target, uintptr_t func, uint32_t index) const noexcept
	{
		return Detours::X64::DetourVTable(target, func, index);
	}

	uintptr_t patch::detour_jump(uintptr_t target, uintptr_t func) const noexcept
	{
		return Detours::X64::DetourFunction(target, func);
	}

	uintptr_t patch::detour_call(uintptr_t target, uintptr_t func) const noexcept
	{
		return Detours::X64::DetourFunction(target, func, Detours::X64Option::USE_REL32_CALL);
	}

	uint32_t patch::calc_rva(uintptr_t from, uintptr_t target, uint32_t opcode_offset) const noexcept
	{
		ptrdiff_t delta = target - (from + sizeof(opcode_offset));
		if (abs(delta) >= INT_MAX) return 0;
		return (uint32_t)delta;
	}

	uintptr_t patch::find_pattern(uintptr_t start_address, uintptr_t max_size, const char* mask) const noexcept
	{
		std::vector<std::pair<uint8_t, bool>> pattern;

		for (size_t i = 0; i < strlen(mask);)
		{
			if (mask[i] != '?')
			{
				pattern.emplace_back((uint8_t)strtoul(&mask[i], nullptr, 16), false);
				i += 3;
			}
			else
			{
				pattern.emplace_back(0x00, true);
				i += 2;
			}
		}

		const uint8_t* dataStart = (uint8_t*)start_address;
		const uint8_t* dataEnd = (uint8_t*)start_address + max_size + 1;

		auto ret = std::search(dataStart, dataEnd, pattern.begin(), pattern.end(),
			[](uint8_t CurrentByte, std::pair<uint8_t, bool>& Pattern) {
				return Pattern.second || (CurrentByte == Pattern.first);
			});

		if (ret == dataEnd)
			return 0;

		return std::distance(dataStart, ret) + start_address;
	}

	std::vector<uintptr_t> patch::find_patterns(uintptr_t start_address, uintptr_t max_size, const char* mask) const noexcept
	{
		std::vector<uintptr_t> results;
		std::vector<std::pair<uint8_t, bool>> pattern;

		for (size_t i = 0; i < strlen(mask);)
		{
			if (mask[i] != '?')
			{
				pattern.emplace_back((uint8_t)strtoul(&mask[i], nullptr, 16), false);
				i += 3;
			}
			else
			{
				pattern.emplace_back(0x00, true);
				i += 2;
			}
		}

		const uint8_t* dataStart = (uint8_t*)start_address;
		const uint8_t* dataEnd = (uint8_t*)start_address + max_size + 1;

		for (const uint8_t* i = dataStart;;)
		{
			auto ret = std::search(i, dataEnd, pattern.begin(), pattern.end(),
				[](uint8_t CurrentByte, std::pair<uint8_t, bool>& Pattern) {
					return Pattern.second || (CurrentByte == Pattern.first);
				});

			if (ret == dataEnd)
				break;

			uintptr_t addr = std::distance(dataStart, ret) + start_address;
			results.push_back(addr);

			i = (uint8_t*)(addr + 1);
		}

		return results;
	}

	bool patch::start_impl() const
	{
		auto name_patch = get_name();

		__try
		{
			auto result = run();
			_MESSAGE("The patch \"%s\" has been %s initialized", name_patch, (result ? "successfully" : "failed"));
			return result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			_ERROR("A serious patch \"%s\" initialization error has occurred", name_patch);
			return false;
		}
	}
}