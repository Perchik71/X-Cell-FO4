// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleThreads : public Module
	{
		UInt32 OldErrMode;
		REL::DetourIAT _functions[2];
		REL::Patch _prologos[2];
	public:
		static constexpr auto SourceName = "Module Threads";

		XCellModuleThreads(void* Context);
		virtual ~XCellModuleThreads() = default;

		XCellModuleThreads(const XCellModuleThreads&) = delete;
		XCellModuleThreads& operator=(const XCellModuleThreads&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}