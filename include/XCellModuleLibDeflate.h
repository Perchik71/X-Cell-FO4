// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleLibDeflate : public Module
	{
		REL::DetourCall _functions[2];
	public:
		static constexpr auto SourceName = "Module LibDeflate";

		XCellModuleLibDeflate(void* Context);
		virtual ~XCellModuleLibDeflate() = default;

		XCellModuleLibDeflate(const XCellModuleLibDeflate&) = delete;
		XCellModuleLibDeflate& operator=(const XCellModuleLibDeflate&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}