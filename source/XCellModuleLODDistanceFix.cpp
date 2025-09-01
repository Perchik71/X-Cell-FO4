// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleLODDistanceFix.h"
#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"

namespace XCell
{
	ModuleLODDistanceFix::ModuleLODDistanceFix(void* Context) :
		Module(Context, SourceName, CVarLODDistance)
	{
		auto gContext = (XCell::Context*)Context;
		auto base = gContext->ProcessBase;

		//
		// It is required because many STATs that are also workshop buildable have LOD
		// If you add the flag, it causes bug above.
		//
		// Sets as always disabled

		_fixes[0].Install(REL::ID(260), { 0x31, 0xC0, 0xC3, 0x90 });
		_fixes[1].Install(REL::ID(261), { 0x31, 0xC0, 0xC3, 0x90 });
		_fixes[2].Install(REL::ID(262), { 0x31, 0xC0, 0xC3, 0x90 });
		_fixes[3].Install(REL::ID(263), { 0xEB, 0x2C, 0x90 });
		_fixes[4].Install(REL::ID(264), { 0xEB, 0x28, 0x90 });
		_fixes[5].Install(REL::ID(265), { 0xEB });
		_fixes[6].Install(REL::ID(266), { 0xC3, 0x90 });

		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			_fixes[7].Install(REL::ID(267), { 0x31, 0xC0, 0xC3, 0x90 });
	}

	HRESULT ModuleLODDistanceFix::InstallImpl()
	{
		_fixes[0].Enable();
		_fixes[1].Enable();
		_fixes[2].Enable();
		_fixes[3].Enable();
		_fixes[4].Enable();
		_fixes[5].Enable();
		_fixes[6].Enable();

		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			_fixes[7].Enable();

		return S_OK;
	}

	HRESULT ModuleLODDistanceFix::ShutdownImpl()
	{
		// Returned

		_fixes[0].Disable();
		_fixes[1].Disable();
		_fixes[2].Disable();
		_fixes[3].Disable();
		_fixes[4].Disable();
		_fixes[5].Disable();
		_fixes[6].Disable();

		if (REL::Version() == RUNTIME_VERSION_1_10_163)
			_fixes[7].Disable();

		return S_OK;
	}
}