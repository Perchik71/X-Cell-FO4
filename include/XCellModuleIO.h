// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleIO : public Module
	{
		REL::DetourIAT _functions[4];
	public:
		static constexpr auto SourceName = "Module IO";

		XCellModuleIO(void* Context);
		virtual ~XCellModuleIO() = default;

		XCellModuleIO(const XCellModuleIO&) = delete;
		XCellModuleIO& operator=(const XCellModuleIO&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}