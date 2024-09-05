// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_loadscreen : public patch
	{
	public:
		patch_loadscreen() = default;
		patch_loadscreen(const patch_loadscreen&) = default;
		virtual ~patch_loadscreen() = default;

		patch_loadscreen& operator=(const patch_loadscreen&) = default;

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	};
}