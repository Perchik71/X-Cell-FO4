// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleThreads : public Module
	{
		UInt32 OldErrMode;
		REL::DetourIAT _functions[2];
		REL::Patch _prologos[2];
	public:
		static constexpr auto SourceName = "Module Threads";

		ModuleThreads(void* Context);
		virtual ~ModuleThreads() = default;

		ModuleThreads(const ModuleThreads&) = delete;
		ModuleThreads& operator=(const ModuleThreads&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}