// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <imgui.h>

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleMemory : public Module
	{
	public:
		static constexpr auto SourceName = "Module Memory";

		XCellModuleMemory(void* Context);
		virtual ~XCellModuleMemory() = default;

		XCellModuleMemory(const XCellModuleMemory&) = delete;
		XCellModuleMemory& operator=(const XCellModuleMemory&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}