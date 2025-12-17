// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModulePackageAllocateLocation.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"

namespace XCell
{
	ModulePackageAllocateLocation::ModulePackageAllocateLocation(void* Context) :
		Module(Context, SourceName, CVarPackageAllocateLocation)
	{
		if ((REL::Version() == RUNTIME_VERSION_1_10_984) || (REL::Version() == RUNTIME_VERSION_1_11_191))
		{
			auto Sub = REL::ID(190);
			_function.Install(REL::ID(191), Sub);
		}
	}

	HRESULT ModulePackageAllocateLocation::InstallImpl()
	{
		if ((REL::Version() != RUNTIME_VERSION_1_10_984) && (REL::Version() != RUNTIME_VERSION_1_11_191))
			return S_FALSE;

		_function.Enable();

		return S_OK;
	}

	HRESULT ModulePackageAllocateLocation::ShutdownImpl()
	{
		// Returned
		
		_function.Disable();

		return S_OK;
	}
}