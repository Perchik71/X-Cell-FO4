// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <vector>
#include <xc_assertion.h>
#include <initializer_list>

namespace xc
{
	class scope_relocate_al
	{
	public:
		inline scope_relocate_al(LPVOID address, SIZE_T size) : _protected(0), _address(address), _size(size)
		{
			_xc_assert_msg_fmt(VirtualProtect(_address, _size, PAGE_EXECUTE_READWRITE, &_protected),
				"Address: %p Size: %X", _address, _size);
		}

		inline ~scope_relocate_al()
		{
			// Ignore if this fails, the memory was copied either way
			VirtualProtect(_address, _size, _protected, &_protected);
		}
	private:
		LPVOID _address;
		SIZE_T _size;
		DWORD _protected;
	};

	class patch
	{
	public:
		patch() = default;
		patch(const patch&) = default;
		virtual ~patch() = default;

		patch& operator=(const patch&) = default;

		bool start();

		virtual const char* get_name() const noexcept = 0;
		virtual bool game_data_ready_handler() const noexcept = 0;
	protected:
		virtual bool run() const = 0;
	
		virtual bool patch_mem_nop(uintptr_t offset, size_t size) const noexcept;
		virtual bool patch_mem(uintptr_t offset, uint8_t* buffer, size_t size) const noexcept;
		virtual bool patch_mem(uintptr_t offset, initializer_list<uint8_t> list) const noexcept;
		virtual uintptr_t patch_iat(HMODULE current_module, const char* import_module_name,
			const char* func_name, uintptr_t func) const noexcept;
		virtual uintptr_t patch_iat_delay(HMODULE current_module, const char* import_module_name,
			const char* func_name, uintptr_t func) const noexcept;
		virtual uintptr_t patch_vtable_func(uintptr_t target, uintptr_t func, uint32_t index) const noexcept;
		virtual uintptr_t detour_jump(uintptr_t target, uintptr_t func) const noexcept;
		virtual uintptr_t detour_call(uintptr_t target, uintptr_t func) const noexcept;
		virtual uint32_t calc_rva(uintptr_t from, uintptr_t target, uint32_t opcode_offset) const noexcept;
		virtual uintptr_t find_pattern(uintptr_t start_address, uintptr_t max_size, const char* mask) const noexcept;
		virtual std::vector<uintptr_t> find_patterns(uintptr_t start_address, uintptr_t max_size, const char* mask) const noexcept;
	private:
		bool start_impl() const;
	};
}