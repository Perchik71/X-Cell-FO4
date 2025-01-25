// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModulePackageAllocateLocation : public Module
	{
		REL::DetourCall _function;
	public:
		static constexpr auto SourceName = "Module Package Allocate Location";

		XCellModulePackageAllocateLocation(void* Context);
		virtual ~XCellModulePackageAllocateLocation() = default;

		XCellModulePackageAllocateLocation(const XCellModulePackageAllocateLocation&) = delete;
		XCellModulePackageAllocateLocation& operator=(const XCellModulePackageAllocateLocation&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}