// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleInitTints.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

#include <f4se/GameForms.h>
#include <f4se/GameObjects.h>

namespace XCell
{
	ModuleInitTints::ModuleInitTints(void* Context) :
		Module(Context, SourceName, CVarInitTints)
	{
		// Removing all checks that block the load for tints by 0x0 plugin and the IsChargenPresent flag.
		useless_code_remove[0].Install(REL::ID(250), 0x35);
		useless_code_remove[1].Install(REL::ID(251), 0x35);
		useless_code_remove[2].Install(REL::ID(253), 0x19);
		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			useless_code_remove[3].Install(REL::ID(255), 0x19);
		
		// There are a lot of checks in this function, it was also used in the facegen verification function.
		// Sets return always 1 or true.		
		check_function.Install(REL::ID(252), { 0xB0, 0x01, 0xC3, 0x90, 0x90 });
	
		// Skipping the entire section, can't delete the pointer, otherwise will go back to where started.
		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			skips_remove_tints.Install(REL::ID(254), { 0xE9, 0x9C, 0x00, 0x00, 0x00, 0x90 });
		else
			skips_remove_tints.Install(REL::ID(254), { 0xE9, 0x7D, 0x00, 0x00, 0x00, 0x90 });
	}

	HRESULT ModuleInitTints::InstallImpl()
	{
		if ((REL::Version() != RUNTIME_VERSION_1_10_984) && (REL::Version() != RUNTIME_VERSION_1_10_163))
			return S_FALSE;

		useless_code_remove[0].Enable();
		useless_code_remove[1].Enable();
		useless_code_remove[2].Enable();
		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			useless_code_remove[3].Enable();

		check_function.Enable();
		skips_remove_tints.Enable();

		return S_OK;
	}

	HRESULT ModuleInitTints::ShutdownImpl()
	{
		// Returned
		
		useless_code_remove[0].Disable();
		useless_code_remove[1].Disable();
		useless_code_remove[2].Disable();
		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			useless_code_remove[3].Disable();

		check_function.Disable();
		skips_remove_tints.Disable();

		return S_OK;
	}
}