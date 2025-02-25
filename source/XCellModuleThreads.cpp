// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <comdef.h>

#include "XCellModuleThreads.h"
#include "XCellCVar.h"
#include "XCellPlugin.h"

namespace XCell
{
	static BOOL WINAPI HKSetThreadPriority(HANDLE Thread, int Priority)
	{
		// Don't allow a priority below normal - Fallout 4 doesn't have many "idle" threads
		return SetThreadPriority(Thread, max(THREAD_PRIORITY_NORMAL, Priority));
	}

	static DWORD_PTR WINAPI HKSetThreadAffinityMask(HANDLE Thread, DWORD_PTR AffinityMask)
	{
		// Don't change anything
		return numeric_limits<DWORD_PTR>::max();
	}

	ModuleThreads::ModuleThreads(void* Context) :
		Module(Context, SourceName, CVarThreads), OldErrMode(0)
	{
		auto gContext = ((XCell::Context*)Context);

		_functions[0].Install(gContext->ProcessBase, "kernel32.dll", "SetThreadPriority", (UInt64)&HKSetThreadPriority);
		_functions[1].Install(gContext->ProcessBase, "kernel32.dll", "SetThreadAffinityMask", (UInt64)&HKSetThreadAffinityMask);

		auto kernel_32 = GetModuleHandleA("kernel32.dll");
		if (kernel_32)
		{
			auto SetPriorityClass_addr = GetProcAddress(kernel_32, "SetPriorityClass");
			auto SetProcessAffinityMask_addr = GetProcAddress(kernel_32, "SetProcessAffinityMask");
			if (SetPriorityClass_addr)
				_prologos[0].Install((UInt64)SetPriorityClass_addr, { 0x31, 0xC0, 0xC3, 0x90, });
			if (SetProcessAffinityMask_addr)
				_prologos[1].Install((UInt64)SetProcessAffinityMask_addr, { 0x31, 0xC0, 0xC3, 0x90, });
		}
	}

	HRESULT ModuleThreads::InstallImpl()
	{
		_functions[0].Enable();
		_functions[1].Enable();

		auto ProcessHandle = GetCurrentProcess();
		if (!SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS))
		{
			auto ErrorLast = GetLastError();
			_ERROR("SetPriorityClass returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
		}
		else
			_MESSAGE("Set high priority has been set for process");

		if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
		{
			auto ErrorLast = GetLastError();
			_ERROR("SetThreadPriority returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
		}
		else
			_MESSAGE("Set high priority has been set for main thread");

		DWORD_PTR processAffinityMask, systemAffinityMask;
		if (!GetProcessAffinityMask(ProcessHandle, &processAffinityMask, &systemAffinityMask))
		{
			auto ErrorLast = GetLastError();
			_ERROR("GetProcessAffinityMask returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
		}
		else
		{
			_MESSAGE("processAffinityMask: 0x%X", processAffinityMask);
			_MESSAGE("systemAffinityMask: 0x%X", systemAffinityMask);

			if (processAffinityMask != systemAffinityMask)
			{
				_MESSAGE("A change in the usage of processor cores has been detected");

				if (!SetProcessAffinityMask(ProcessHandle, systemAffinityMask))
				{
					auto ErrorLast = GetLastError();
					_ERROR("SetProcessAffinityMask returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
				}
				else
					_MESSAGE("Restore usage of processor cores");
			}

			// Complete removal of WinAPI functions SetPriorityClass and SetProcessAffinityMask.
			// Protection against premeditated, foolishly committed spoilage of the process.
			_prologos[0].Enable();
			_prologos[1].Enable();
		}

		// The system does not display the critical-error-handler message box. 
		// Instead, the system sends the error to the calling process.
		// Best practice is that all applications call the process - wide SetErrorMode 
		// function with a parameter of SEM_FAILCRITICALERRORS at startup.
		// This is to prevent error mode dialogs from hanging the application.
		OldErrMode = 0;
		if (!SetThreadErrorMode(SEM_FAILCRITICALERRORS, &OldErrMode))
		{
			auto ErrorLast = GetLastError();
			_ERROR("SetThreadErrorMode returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
		}

		return S_OK;
	}

	HRESULT ModuleThreads::ShutdownImpl()
	{
		// Returned

		_functions[0].Disable();
		_functions[1].Disable();

		_prologos[0].Disable();
		_prologos[1].Disable();

		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		SetThreadErrorMode(OldErrMode, &OldErrMode);

		return S_OK;
	}
}