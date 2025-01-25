// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleLoadScreen.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"

namespace XCell
{
	XCellModuleLoadScreen::XCellModuleLoadScreen(void* Context) :
		Module(Context, SourceName, CVarLoadScreen)
	{
		_fixes.Install(REL::ID(170), { 0xC3, 0x90 });
	}

	HRESULT XCellModuleLoadScreen::InstallImpl()
	{
		// So that it is never called
		// OG pattern 488945??488B05????????48897D??8B88????????448BFE8975??488BDE85C974??488B98????????488BC3488D34CB483BDE74??
		_fixes.Enable();

		return S_OK;
	}

	HRESULT XCellModuleLoadScreen::ShutdownImpl()
	{
		// Returned
		
		_fixes.Disable();

		return S_OK;
	}
}