// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleLoadScreen : public Module
	{
		REL::Patch _fixes;
	public:
		static constexpr auto SourceName = "Module LoadScreen";

		ModuleLoadScreen(void* Context);
		virtual ~ModuleLoadScreen() = default;

		ModuleLoadScreen(const ModuleLoadScreen&) = delete;
		ModuleLoadScreen& operator=(const ModuleLoadScreen&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}