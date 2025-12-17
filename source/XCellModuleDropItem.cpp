// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/ScaleformMovie.h>
#include <f4se/ScaleformValue.h>

#include "XCellModuleDropItem.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

namespace XCell
{
	ModuleDropItem::ModuleDropItem(void* Context) :
		Module(Context, SourceName, CVarDropItem)
	{}

	HRESULT ModuleDropItem::InstallImpl()
	{
		if ((REL::Version() != RUNTIME_VERSION_1_10_984) && (REL::Version() != RUNTIME_VERSION_1_11_191))
			return S_FALSE;

		// Extended sets new ammo value
		REL::Impl::Patch(REL::ID(270), { 0x89, 0xEA, 0x90 });
		REL::Impl::Patch(REL::ID(271), { 0x89, 0x51, 0x18, 0x90 });
		REL::Impl::Patch(REL::ID(272), { 0x8B, 0x57, 0x18, 0x90 });
		REL::Impl::Patch(REL::ID(273), { 0x44, 0x89, 0xF2, 0x90 });
		REL::Impl::Patch(REL::ID(274), { 0x44, 0x89, 0xF2, 0x90 });
		REL::Impl::Patch(REL::ID(275), { 0x8B, 0x54, 0x30, 0x10, 0x90 });
		REL::Impl::Patch(REL::ID(276), { 0x8B, 0x51, 0x10, 0x90 });
		REL::Impl::Patch(REL::ID(277), { 0x8B, 0x55, 0x7F, 0x90 });
		REL::Impl::Patch(REL::ID(278), { 0x89, 0xD5, 0x90 });
		REL::Impl::Patch(REL::ID(279), { 0x45, 0x8B, 0x75, 0x18, 0x90 });
		REL::Impl::Patch(REL::ID(280), { 0x83, 0xFA, 0x01, 0x90 });
		REL::Impl::Patch(REL::ID(281), { 0x89, 0x68, 0x18, 0x90 });

		// Gets
		REL::Impl::Patch(REL::ID(282), { 0x8B, 0x40, 0x18, 0x90 });
		REL::Impl::Patch(REL::ID(283), { 0x41, 0x89, 0xC1, 0x90 });

		// Skips 0x7FFF
		REL::Impl::Patch(REL::ID(284), { 0x89, 0xF2, 0x90, 0x90 });
		REL::Impl::Patch(REL::ID(285), { 0x89, 0xC2, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });

		// Extended check (correct drop item model)
		REL::Impl::Patch(REL::ID(286), { 0x89, 0xC5, 0x90 });
		REL::Impl::Patch(REL::ID(287), { 0x41, 0x83, 0xF8, 0x01, 0x90 });
		REL::Impl::Patch(REL::ID(288), { 0x41, 0x89, 0xC0, 0x90 });

		if ((REL::Version() == RUNTIME_VERSION_1_11_191) || !GetModuleHandleA("MentatsF4SE.dll"))
			// Many items fix
			REL::Impl::Patch(REL::ID(289), { 0x44, 0x8B, 0x44, 0x24, 0x70, 0x90 });

		return S_OK;
	}

	HRESULT ModuleDropItem::ShutdownImpl()
	{
		return S_OK;
	}
}