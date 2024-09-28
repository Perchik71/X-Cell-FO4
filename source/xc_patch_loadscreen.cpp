// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

//#include <f4se/GameForms.h>
//#include <f4se/GameData.h>
#include <xc_patch_loadscreen.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	const char* patch_loadscreen::get_name() const noexcept
	{
		return "loadscreen";
	}

	bool patch_loadscreen::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool patch_loadscreen::run() const
	{
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
			// So that it is never called
			// pattern 488945??488B05????????48897D??8B88????????448BFE8975??488BDE85C974??488B98????????488BC3488D34CB483BDE74??
			patch_mem((g_plugin->get_base() + 0x12989E0), { 0xC3, 0x90 });	// Removing an item from the load logo, any
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
			// So that it is never called
			patch_mem((g_plugin->get_base() + 0xFE23C0), { 0xC3, 0x90 });	// Removing an item from the load logo, any
		else
		{
			_ERROR("The patch \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
	}
}