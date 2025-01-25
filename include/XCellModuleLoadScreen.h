// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleLoadScreen : public Module
	{
		REL::Patch _fixes;
	public:
		static constexpr auto SourceName = "Module LoadScreen";

		XCellModuleLoadScreen(void* Context);
		virtual ~XCellModuleLoadScreen() = default;

		XCellModuleLoadScreen(const XCellModuleLoadScreen&) = delete;
		XCellModuleLoadScreen& operator=(const XCellModuleLoadScreen&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}