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
	class ModuleMemory : public Module
	{
	public:
		static constexpr auto SourceName = "Module Memory";

		ModuleMemory(void* Context);
		virtual ~ModuleMemory() = default;

		ModuleMemory(const ModuleMemory&) = delete;
		ModuleMemory& operator=(const ModuleMemory&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}