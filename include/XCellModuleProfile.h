// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleProfile : public Module
	{
	public:
		static constexpr auto SourceName = "Module Profile";

		ModuleProfile(void* Context);
		virtual ~ModuleProfile() = default;

		ModuleProfile(const ModuleProfile&) = delete;
		ModuleProfile& operator=(const ModuleProfile&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}