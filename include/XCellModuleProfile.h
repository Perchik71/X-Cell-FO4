// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleProfile : public Module
	{
	public:
		static constexpr auto SourceName = "Module Profile";

		XCellModuleProfile(void* Context);
		virtual ~XCellModuleProfile() = default;

		XCellModuleProfile(const XCellModuleProfile&) = delete;
		XCellModuleProfile& operator=(const XCellModuleProfile&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}