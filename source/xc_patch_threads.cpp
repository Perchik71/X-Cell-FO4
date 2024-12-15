// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <comdef.h>
#include <xc_patch_threads.h>

namespace xc
{
	const char* patch_threads::get_name() const noexcept
	{
		return "threads";
	}

	bool patch_threads::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool patch_threads::run() const
	{
		auto base = GetModuleHandleA(NULL);

#if 0
		// freezing

		patch_iat(base, "kernel32.dll", "Sleep", (uintptr_t)&sleep);
		patch_iat(base, "kernel32.dll", "SleepEx", (uintptr_t)&sleep_ex);
#endif

		patch_iat(base, "kernel32.dll", "SetThreadPriority", (uintptr_t)&set_thread_priority);
		patch_iat(base, "kernel32.dll", "SetThreadAffinityMask", (uintptr_t)&set_thread_affinity_mask);

		auto ProcessHandle = GetCurrentProcess();
		if (!SetPriorityClass(ProcessHandle, /*ABOVE_NORMAL_PRIORITY_CLASS*/ HIGH_PRIORITY_CLASS))
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
			auto kernel_32 = GetModuleHandle("kernel32.dll");
			if (kernel_32)
			{
				auto SetPriorityClass_addr = GetProcAddress(kernel_32, "SetPriorityClass");
				auto SetProcessAffinityMask_addr = GetProcAddress(kernel_32, "SetProcessAffinityMask");
				if (SetPriorityClass_addr)
					patch_mem((uintptr_t)SetPriorityClass_addr, { 0x31, 0xC0, 0xC3, 0x90, });
				if (SetProcessAffinityMask_addr)
					patch_mem((uintptr_t)SetProcessAffinityMask_addr, { 0x31, 0xC0, 0xC3, 0x90, });
			}
		}

		patch_iat(base, "kernel32.dll", "SetThreadAffinityMask", (uintptr_t)&set_thread_affinity_mask);

		// The system does not display the critical-error-handler message box. 
		// Instead, the system sends the error to the calling process.
		// Best practice is that all applications call the process - wide SetErrorMode 
		// function with a parameter of SEM_FAILCRITICALERRORS at startup.
		// This is to prevent error mode dialogs from hanging the application.
		UInt32 OldErrMode = 0;
		if (!SetThreadErrorMode(SEM_FAILCRITICALERRORS, &OldErrMode))
		{
			auto ErrorLast = GetLastError();
			_ERROR("SetThreadErrorMode returned failed (0x%x): %s", ErrorLast, _com_error(ErrorLast).ErrorMessage());
		}

		return true;
	}

#if 0
	uint32_t patch_threads::sleep(uint32_t ms)
	{
		// Bethesda's spinlock calls Sleep(0) every iteration until 10,000. Then it
		// uses Sleep(1). Even with 0ms waits, there's a tiny performance penalty.
		if (ms == 0) return 0;

		return SleepEx(ms, FALSE);
	}

	uint32_t patch_threads::sleep_ex(uint32_t ms, bool alterable)
	{
		// Bethesda's spinlock calls Sleep(0) every iteration until 10,000. Then it
		// uses Sleep(1). Even with 0ms waits, there's a tiny performance penalty.
		if (ms == 0) return 0;

		return SleepEx(ms, alterable);
	}
#endif

	bool patch_threads::set_thread_priority(HANDLE thread_handle, int priority)
	{
		// Don't allow a priority below normal - Fallout 4 doesn't have many "idle" threads
		return SetThreadPriority(thread_handle, std::max(THREAD_PRIORITY_NORMAL, priority));
	}

	uintptr_t patch_threads::set_thread_affinity_mask(HANDLE thread_handle, uintptr_t affinity_mask)
	{
		// Don't change anything
		return 0xFFFFFFFF;
	}
}