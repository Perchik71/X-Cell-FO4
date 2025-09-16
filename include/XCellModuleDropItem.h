// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleDropItem : public Module
	{
		REL::Patch _fixes;
		REL::DetourCall _function;
	public:
		static constexpr auto SourceName = "Module DropItem";

		ModuleDropItem(void* Context);
		virtual ~ModuleDropItem() = default;

		ModuleDropItem(const ModuleDropItem&) = delete;
		ModuleDropItem& operator=(const ModuleDropItem&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}