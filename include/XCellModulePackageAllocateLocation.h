// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModulePackageAllocateLocation : public Module
	{
		REL::DetourCall _function;
	public:
		static constexpr auto SourceName = "Module Package Allocate Location";

		ModulePackageAllocateLocation(void* Context);
		virtual ~ModulePackageAllocateLocation() = default;

		ModulePackageAllocateLocation(const ModulePackageAllocateLocation&) = delete;
		ModulePackageAllocateLocation& operator=(const ModulePackageAllocateLocation&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}