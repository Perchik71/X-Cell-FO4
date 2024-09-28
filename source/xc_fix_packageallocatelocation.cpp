// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_fix_packageallocatelocation.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	uintptr_t pointer_package_allocate_location_sub = 0;

	void* fix_package_allocate_location::sub(void* self)
	{
		return self ? fastCall<void*>(pointer_package_allocate_location_sub, self) : nullptr;
	}

	const char* fix_package_allocate_location::get_name() const noexcept
	{
		return "package_allocate_location";
	}

	bool fix_package_allocate_location::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool fix_package_allocate_location::run() const
	{
		// only NG
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			pointer_package_allocate_location_sub = g_plugin->get_base() + 0x2352E0;
			detour_call(g_plugin->get_base() + 0x711CE4, (uintptr_t)&sub);
		}
		else
		{
			_ERROR("The fix \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
	}
}