// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/ScaleformMovie.h>
#include <xc_patch.h>

namespace xc
{
	class fix_greymovies : public patch
	{
	public:
		fix_greymovies() = default;
		fix_greymovies(const fix_greymovies&) = default;
		virtual ~fix_greymovies() = default;

		fix_greymovies& operator=(const fix_greymovies&) = default;

		static void setbgalpha(GFxMovieView* self, float);

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	};
}