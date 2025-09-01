// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleLODDistanceFix : public Module
	{
		REL::Patch _fixes[8];
	public:
		static constexpr auto SourceName = "Module LOD Distance Fix";

		ModuleLODDistanceFix(void* Context);
		virtual ~ModuleLODDistanceFix() = default;

		ModuleLODDistanceFix(const ModuleLODDistanceFix&) = delete;
		ModuleLODDistanceFix& operator=(const ModuleLODDistanceFix&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}