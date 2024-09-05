// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/GameForms.h>
#include <xc_patch.h>

namespace xc
{
	class patch_facegen : public patch
	{
	public:
		patch_facegen() = default;
		patch_facegen(const patch_facegen&) = default;
		virtual ~patch_facegen() = default;

		patch_facegen& operator=(const patch_facegen&) = default;

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	private:
		inline static bool (*path_printf_facegen)(TESNPC* npc_form, char* buffer, size_t buffer_size, int index_texture);
		static bool can_use_preprocessing_head(TESNPC* npc_form);
	};
}