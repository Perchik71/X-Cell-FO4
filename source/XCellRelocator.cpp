// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se_common/Relocation.h>
#include <detours/Detours.h>

#include "XCellRelocator.h"

namespace XCell
{
	namespace REL
	{
		namespace Impl
		{
			UInt64 __stdcall FindPattern(UInt64 Address, UInt64 MaxSize, const char* Mask)
			{
				vector<pair<UInt8, bool>> pattern;

				for (size_t i = 0; i < strlen(Mask);)
				{
					if (Mask[i] != '?')
					{
						pattern.emplace_back((UInt8)strtoul(&Mask[i], nullptr, 16), false);
						i += 3;
					}
					else
					{
						pattern.emplace_back(0x00, true);
						i += 2;
					}
				}

				const UInt8* dataStart = (UInt8*)Address;
				const UInt8* dataEnd = (UInt8*)Address + MaxSize + 1;

				auto ret = search(dataStart, dataEnd, pattern.begin(), pattern.end(),
					[](UInt8 CurrentByte, pair<UInt8, bool>& Pattern) {
						return Pattern.second || (CurrentByte == Pattern.first);
					});

				if (ret == dataEnd)
					return 0;

				return (UInt64)(distance(dataStart, ret) + Address);
			}

			vector<UInt64> __stdcall FindPatterns(UInt64 Address, UInt64 MaxSize, const char* Mask)
			{
				vector<uintptr_t> results;
				vector<pair<UInt8, bool>> pattern;

				for (size_t i = 0; i < strlen(Mask);)
				{
					if (Mask[i] != '?')
					{
						pattern.emplace_back((uint8_t)strtoul(&Mask[i], nullptr, 16), false);
						i += 3;
					}
					else
					{
						pattern.emplace_back(0x00, true);
						i += 2;
					}
				}

				const UInt8* dataStart = (UInt8*)Address;
				const UInt8* dataEnd = (UInt8*)Address + MaxSize + 1;

				for (const UInt8* i = dataStart;;)
				{
					auto ret = search(i, dataEnd, pattern.begin(), pattern.end(),
						[](UInt8 CurrentByte, pair<UInt8, bool>& Pattern) {
							return Pattern.second || (CurrentByte == Pattern.first);
						});

					if (ret == dataEnd)
						break;

					uintptr_t addr = distance(dataStart, ret) + Address;
					results.push_back(addr);

					i = (uint8_t*)(addr + 1);
				}

				return results;
			}

			void __stdcall Patch(UInt64 Target, UInt8* Data, size_t Size)
			{
				if (!Target || !Data || !Size) return;
				ScopeLock Lock(Target, Size);
				if (Lock.HasUnlocked())
				{
					for (UInt64 i = Target; i < (Target + Size); i++)
						*(volatile UInt8*)i = *Data++;
				}
			}

			void __stdcall Patch(UInt64 Target, initializer_list<UInt8> Data)
			{
				if (!Target || !Data.size()) return;
				ScopeLock Lock(Target, Data.size());
				if (Lock.HasUnlocked())
				{
					UInt64 i = Target;
					for (auto value : Data)
						*(volatile UInt8*)i++ = value;
				}
			}

			void __stdcall PatchNop(UInt64 Target, size_t Size)
			{
				if (!Target || !Size) return;
				ScopeLock Lock(Target, Size);
				if (Lock.HasUnlocked())
					memset((LPVOID)Target, 0x90, Size);
			}

			UInt64 __stdcall DetourJump(UInt64 Target, UInt64 Function)
			{
				if (!Target || !Function) return 0;
				return Detours::X64::DetourFunction(Target, Function, Detours::X64Option::USE_REL32_JUMP);
			}

			UInt64 __stdcall DetourCall(UInt64 Target, UInt64 Function)
			{
				if (!Target || !Function) return 0;
				return (UInt64)Detours::X64::DetourFunction(Target, Function, Detours::X64Option::USE_REL32_CALL);
			}

			UInt64 __stdcall DetourVTable(UInt64 Target, UInt64 Function, UInt32 Index)
			{
				if (!Target || !Function) return 0;
				return (UInt64)Detours::X64::DetourVTable(Target, Function, Index);
			}

			UInt64 __stdcall DetourIAT(UInt64 TargetModule, const char* Import, const char* FunctionName, UInt64 Function)
			{
				if (!TargetModule || !Function || !Import || !FunctionName) return 0;
				return (UInt64)Detours::IATHook(TargetModule, Import, FunctionName, Function);
			}

			UInt64 __stdcall DetourIATDelayed(UInt64 TargetModule, const char* Import, const char* FunctionName, UInt64 Function)
			{
				if (!TargetModule || !Function || !Import || !FunctionName) return 0;
				return (UInt64)Detours::IATDelayedHook(TargetModule, Import, FunctionName, Function);
			}
		}

		// HookInterface

		HookInterface::HookInterface() :
			_enabled(false)
		{}

		HookInterface::~HookInterface()
		{
			Disable();
		}

		HRESULT HookInterface::Enable() noexcept(true)
		{
			if (!_enabled)
			{
				auto Result = EnableImpl();
				_enabled = SUCCEEDED(Result);
				return Result;
			}

			return S_FALSE;
		}

		HRESULT HookInterface::Disable() noexcept(true)
		{
			if (_enabled)
			{
				auto Result = DisableImpl();
				_enabled = FAILED(Result);
				return Result;
			}

			return S_FALSE;
		}

		// ScopeLock

		ScopeLock::ScopeLock(UInt64 Target, UInt64 Size) :
			_locked(false), _old(0), _target(Target), _size(Size)
		{
			_locked = VirtualProtect(reinterpret_cast<void*>(Target), (SIZE_T)Size, PAGE_EXECUTE_READWRITE, (PDWORD)&_old);
		}

		ScopeLock::ScopeLock(void* Target, UInt64 Size) :
			_locked(false), _old(0), _target((UInt64)Target), _size(Size)
		{
			_locked = VirtualProtect(Target, (SIZE_T)Size, PAGE_EXECUTE_READWRITE, (PDWORD)&_old);
		}

		ScopeLock::~ScopeLock()
		{
			if (_locked)
			{
				// Ignore if this fails, the memory was copied either way
				VirtualProtect(reinterpret_cast<void*>(_target), (SIZE_T)_size, _old, &_old);
				FlushInstructionCache(GetCurrentProcess(), (LPVOID)_target, _size);
				_locked = false;
			}
		}

		// Patch

		Patch::Patch() :
			HookInterface(), _size(0), _target(0)
		{}

		bool Patch::Install(UInt64 Target, const UInt8* Buffer, UInt64 Size) noexcept(true)
		{
			if (!Target || !Buffer || !Size)
				return false;

			_old = std::make_unique<UInt8[]>(Size);
			_new = std::make_unique<UInt8[]>(Size);

			if (!_new || !_old)
				return false;

			memcpy(_new.get(), Buffer, Size);

			_target = Target;
			_size = Size;
			
			return true;
		}

		bool Patch::Install(UInt64 Target, const initializer_list<UInt8>& Buffer) noexcept(true)
		{
			if (!Target || !Buffer.size())
				return false;

			_old = std::make_unique<UInt8[]>(Buffer.size());
			_new = std::make_unique<UInt8[]>(Buffer.size());

			if (!_new || !_old)
				return false;

			memcpy(_new.get(), (void*)(&(*Buffer.begin())), Buffer.size());

			_target = Target;
			_size = Buffer.size();

			return true;
		}

		HRESULT Patch::EnableImpl() noexcept(true)
		{
			if (!_size || !_target)
				return S_FALSE;

			if (!_new || !_old)
				return E_FAIL;

			auto pvTarget = reinterpret_cast<void*>(_target);
			auto pvNewMemory = reinterpret_cast<const void*>(_new.get());
			auto pvOldMemory = reinterpret_cast<void*>(_old.get());

			ScopeLock Locker(pvTarget, (SIZE_T)_size);
			if (!Locker.HasUnlocked())
				return E_FAIL;

			memcpy(pvOldMemory, pvTarget, _size);
			memcpy(pvTarget, pvNewMemory, _size);	

			return S_OK;
		}

		HRESULT Patch::DisableImpl() noexcept(true)
		{
			if (!_size || !_target)
				return S_FALSE;

			if (!_new || !_old)
				return E_FAIL;

			auto pvTarget = reinterpret_cast<void*>(_target);
			auto pvMemory = reinterpret_cast<const void*>(_old.get());

			ScopeLock Locker(pvTarget, (SIZE_T)_size);
			if (!Locker.HasUnlocked())
				return E_FAIL;

			memcpy(pvTarget, pvMemory, _size);

			return S_OK;
		}

		// PatchNop

		PatchNop::PatchNop() :
			HookInterface(), _size(0), _target(0)
		{}

		bool PatchNop::Install(UInt64 Target, UInt64 Size) noexcept(true)
		{
			if (!Target || !Size)
				return false;

			_old = std::make_unique<UInt8[]>(Size);
			if (!_old)
				return false;

			_target = Target;
			_size = Size;

			return true;
		}

		HRESULT PatchNop::EnableImpl() noexcept(true)
		{
			if (!_size || !_target)
				return S_FALSE;

			if (!_old)
				return E_FAIL;

			auto pvTarget = reinterpret_cast<UInt8*>(_target);
			auto pvOldMemory = reinterpret_cast<void*>(_old.get());

			ScopeLock Locker(pvTarget, (SIZE_T)_size);
			if (!Locker.HasUnlocked())
				return E_FAIL;

			memcpy(pvOldMemory, pvTarget, _size);
			memset(pvTarget, 0x90, _size);

			return S_OK;
		}

		HRESULT PatchNop::DisableImpl() noexcept(true)
		{
			if (!_size || !_target)
				return S_FALSE;

			if (!_old)
				return E_FAIL;

			auto pvTarget = reinterpret_cast<void*>(_target);
			auto pvMemory = reinterpret_cast<const void*>(_old.get());

			ScopeLock Locker(pvTarget, (SIZE_T)_size);
			if (!Locker.HasUnlocked())
				return E_FAIL;

			memcpy(pvTarget, pvMemory, _size);

			return S_OK;
		}

		// DetourJump

		DetourJump::DetourJump() :
			HookInterface(), _target(0), _function(0), _trampoline(0)
		{}

		bool DetourJump::Install(UInt64 Target, UInt64 Function) noexcept(true)
		{
			if (!Function || !Target)
				return false;

			_target = Target;
			_function = Function;

			return true;
		}

		HRESULT DetourJump::EnableImpl() noexcept(true)
		{
			if (!_function || !_target)
				return S_FALSE;

			_trampoline = Impl::DetourJump(_target, _function);

			return S_OK;
		}

		HRESULT DetourJump::DisableImpl() noexcept(true)
		{
			if (!_trampoline || !_target)
				return S_FALSE;

			if (!Detours::X64::DetourRemove((uintptr_t)_trampoline))
				return E_FAIL;

			return S_OK;
		}

		// DetourCall

		DetourCall::DetourCall() :
			HookInterface(), _target(0), _function(0), _trampoline(0)
		{}

		bool DetourCall::Install(UInt64 Target, UInt64 Function) noexcept(true)
		{
			if (!Function || !Target)
				return false;

			_target = Target;
			_function = Function;

			return true;
		}

		HRESULT DetourCall::EnableImpl() noexcept(true)
		{
			if (!_function || !_target)
				return S_FALSE;

			_trampoline = Impl::DetourCall(_target, _function);

			return S_OK;
		}

		HRESULT DetourCall::DisableImpl() noexcept(true)
		{
			if (!_trampoline || !_target)
				return S_FALSE;

			if (!Detours::X64::DetourRemove((uintptr_t)_trampoline))
				return E_FAIL;

			return S_OK;
		}

		// DetourIAT

		DetourIAT::DetourIAT() :
			HookInterface(), _function(0), _old_function(0), _module(0)
		{}

		bool DetourIAT::Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true)
		{
			if (!Function || !TargetModule || !Import || !FunctionName)
				return false;

			_module = TargetModule;
			_import = Import;
			_function_name = FunctionName;
			_function = Function;

			return true;
		}

		HRESULT DetourIAT::EnableImpl() noexcept(true)
		{
			if (!_function || !_module || _import.empty() || _function_name.empty())
				return S_FALSE;

			_old_function = Impl::DetourIAT(_module, _import.c_str(), _function_name.c_str(), _function);
			return _old_function ? S_OK : E_FAIL;
		}

		HRESULT DetourIAT::DisableImpl() noexcept(true)
		{
			if (!_old_function || !_module || _import.empty() || _function_name.empty())
				return S_FALSE;

			Impl::DetourIAT(_module, _import.c_str(), _function_name.c_str(), _old_function);

			return S_OK;
		}

		// DetourIATDelay

		DetourIATDelay::DetourIATDelay() :
			HookInterface(), _function(0), _old_function(0), _module(0)
		{}

		bool DetourIATDelay::Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true)
		{
			if (!Function || !TargetModule || !Import || !FunctionName)
				return false;

			_module = TargetModule;
			_import = Import;
			_function_name = FunctionName;
			_function = Function;

			return true;
		}

		HRESULT DetourIATDelay::EnableImpl() noexcept(true)
		{
			if (!_function || !_module || _import.empty() || _function_name.empty())
				return S_FALSE;

			_old_function = Impl::DetourIATDelayed(_module, _import.c_str(), _function_name.c_str(), _function);
			return _old_function ? S_OK : E_FAIL;
		}

		HRESULT DetourIATDelay::DisableImpl() noexcept(true)
		{
			if (!_old_function || !_module || _import.empty() || _function_name.empty())
				return S_FALSE;

			Impl::DetourIATDelayed((uintptr_t)_module, _import.c_str(), _function_name.c_str(), _old_function);

			return S_OK;
		}

		// DetourVTable

		DetourVTable::DetourVTable() :
			HookInterface(), _target(0), _function(0), _old_function(0), _index(0)
		{}

		bool DetourVTable::Install(UInt64 Target, UInt64 Function, UInt32 Index) noexcept(true)
		{
			if (!Function || !Target)
				return false;

			_target = Target;
			_function = Function;
			_index = Index;

			return true;
		}

		HRESULT DetourVTable::EnableImpl() noexcept(true)
		{
			if (!_target || !_function)
				return S_FALSE;

			_old_function = Impl::DetourVTable(_target, _function, _index);
			return _old_function ? S_OK : E_FAIL;
		}

		HRESULT DetourVTable::DisableImpl() noexcept(true)
		{
			if (!_old_function || !_target)
				return S_FALSE;

			Impl::DetourVTable((uintptr_t)_target, (uintptr_t)_old_function, (uint32_t)_index);

			return S_OK;
		}

		/////////////////////////////////////////////////////

		UInt64 __stdcall Offset(UInt32 RVA)
		{
			return (UInt64)(RelocationManager::s_baseAddr + RVA);
		}
	}
}