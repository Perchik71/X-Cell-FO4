// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <memory>
#include <vector>

namespace XCell
{
	namespace REL
	{
		namespace Impl
		{
			UInt64 __stdcall FindPattern(UInt64 Address, UInt64 MaxSize, const char* Mask);

			vector<UInt64> __stdcall FindPatterns(UInt64 Address, UInt64 MaxSize, const char* Mask);

			void __stdcall Patch(UInt64 Target, UInt8* Data, size_t Size);

			void __stdcall Patch(UInt64 Target, initializer_list<UInt8> daDatata);

			void __stdcall PatchNop(UInt64 Target, size_t Size);

			UInt64 __stdcall DetourJump(UInt64 Target, UInt64 Function);

			UInt64 __stdcall DetourCall(UInt64 Target, UInt64 Function);

			UInt64 __stdcall DetourVTable(UInt64 Target, UInt64 Function, UInt32 Index);

			UInt64 __stdcall DetourIAT(UInt64 TargetModule, const char* Import, const char* FunctionName, UInt64 Function);

			UInt64 __stdcall DetourIATDelayed(UInt64 TargetModule, const char* Import, const char* FunctionName, UInt64 Function);
		}
		
		class HookInterface
		{
			bool _enabled;
		public:
			HookInterface();
			virtual ~HookInterface();

			virtual HRESULT Enable() noexcept(true);
			virtual HRESULT Disable() noexcept(true);

			[[nodiscard]] inline virtual bool HasEnabled() const noexcept(true) { return _enabled; }
		protected:
			virtual HRESULT EnableImpl() noexcept(true) = 0;
			virtual HRESULT DisableImpl() noexcept(true) = 0;
		};

		class ScopeLock
		{
			bool _locked;
			UInt32 _old;
			UInt64 _target, _size;
		public:
			ScopeLock(UInt64 Target, UInt64 Size);
			ScopeLock(void* Target, UInt64 Size);
			virtual ~ScopeLock();

			[[nodiscard]] inline virtual bool HasUnlocked() const noexcept(true) { return _locked; }
		};

		class Patch : public HookInterface
		{
			unique_ptr<UInt8[]> _old, _new;
			UInt64 _size, _target;
		public:
			Patch();

			virtual bool Install(UInt64 Target, const UInt8* Buffer, UInt64 Size) noexcept(true);
			virtual bool Install(UInt64 Target, const initializer_list<UInt8>& Buffer) noexcept(true);

			Patch(const Patch&) = delete;
			Patch& operator=(const Patch&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		class PatchNop : public HookInterface
		{
			unique_ptr<UInt8[]> _old;
			UInt64 _size, _target;
		public:
			PatchNop();

			virtual bool Install(UInt64 Target, UInt64 Size) noexcept(true);

			PatchNop(const PatchNop&) = delete;
			PatchNop& operator=(const PatchNop&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};
	
		class DetourJump : public HookInterface
		{
			UInt64 _target, _function, _trampoline;
		public:
			DetourJump();

			virtual bool Install(UInt64 Target, UInt64 Function) noexcept(true);
			template<class FunctionType> inline bool Install(UInt64 Target, FunctionType Function) noexcept(true)
			{
				return Install(Target, *(UInt64*)&Function);
			}

			DetourJump(const DetourJump&) = delete;
			DetourJump& operator=(const DetourJump&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		class DetourCall : public HookInterface
		{
			UInt64 _target, _function, _trampoline;
		public:
			DetourCall();

			virtual bool Install(UInt64 Target, UInt64 Function) noexcept(true);
			template<class FunctionType> inline bool Install(UInt64 Target, FunctionType Function) noexcept(true)
			{
				return Install(Target, *(UInt64*)&Function);
			}

			DetourCall(const DetourCall&) = delete;
			DetourCall& operator=(const DetourCall&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		class DetourIAT : public HookInterface
		{
			string _import, _function_name;
			UInt64 _module, _function, _old_function;
		public:
			DetourIAT();

			virtual bool Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true);
			inline virtual bool Install(HMODULE TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true)
			{
				return Install((UInt64)TargetModule, Import, FunctionName, Function);
			}
			template<class FunctionType> inline bool Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName, 
				FunctionType Function) noexcept(true)
			{
				return Install((UInt64)TargetModule, Import, FunctionName, *(UInt64*)&Function);
			}

			DetourIAT(const DetourIAT&) = delete;
			DetourIAT& operator=(const DetourIAT&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		class DetourIATDelay : public HookInterface
		{
			string _import, _function_name;
			UInt64 _module, _function, _old_function;
		public:
			DetourIATDelay();

			virtual bool Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true);
			inline virtual bool Install(HMODULE TargetModule, LPCSTR Import, LPCSTR FunctionName, UInt64 Function) noexcept(true)
			{
				return Install((UInt64)TargetModule, Import, FunctionName, Function);
			}
			template<class FunctionType> inline bool Install(UInt64 TargetModule, LPCSTR Import, LPCSTR FunctionName,
				FunctionType Function) noexcept(true)
			{
				return Install((UInt64)TargetModule, Import, FunctionName, *(UInt64*)&Function);
			}

			DetourIATDelay(const DetourIATDelay&) = delete;
			DetourIATDelay& operator=(const DetourIATDelay&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		class DetourVTable : public HookInterface
		{
			UInt64 _target, _function, _old_function;
			UInt32 _index;
		public:
			DetourVTable();

			virtual bool Install(UInt64 Target, UInt64 Function, UInt32 Index) noexcept(true);
			template<class FunctionType> inline bool Install(UInt64 Target, FunctionType Function, UInt32 Index) noexcept(true)
			{
				return Install(Target, *(UInt64*)&Function, Index);
			}

			DetourVTable(const DetourVTable&) = delete;
			DetourVTable& operator=(const DetourVTable&) = delete;
		protected:
			virtual HRESULT EnableImpl() noexcept(true);
			virtual HRESULT DisableImpl() noexcept(true);
		};

		UInt64 __stdcall Offset(UInt32 RVA);
	}
}
