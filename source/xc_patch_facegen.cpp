// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#define XC_NO_ORIG_SKIP_FACEGEN

#include <f4se/GameObjects.h>
#include <xc_patch_facegen.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	const char* patch_facegen::get_name() const noexcept
	{
		return "facegen";
	}

	bool patch_facegen::run() const
	{
		// The problem is, the game uses facegen, which lies only in fallout4.esm.

		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// 163

			// Working buried function.
			*(uintptr_t*)&path_printf_facegen = g_plugin->get_base() + 0x5B57F0;
			
			auto offset = g_plugin->get_base() + 0x679910;
			// Remove useless stuff.
			patch_mem_nop(offset, 0x1F);
			patch_mem_nop(offset + 0x25, 0x37);

			// mov rcx, r13
			// lea rdx, qword ptr ss:[rbp-0x10]
			// mov r8d, 0x104
			// mov r9d, esi
			// cmp r9d, 0x2
			// mov eax, 0x7
			// cmove r9d, eax
			patch_mem(offset + 0x30, { 0x4C, 0x89, 0xE9, 0x48, 0x8D, 0x55, 0xF0, 0x41, 0xB8, 0x04, 0x01, 0x00, 0x00,
				0x41, 0x89, 0xF1, 0x41, 0x83, 0xF9, 0x02, 0xB8, 0x07, 0x00, 0x00, 0x00, 0x44, 0x0F, 0x44, 0xC8 });

			detour_call(offset + 0x4D, (uintptr_t)path_printf_facegen);

#if defined(XC_NO_ORIG_SKIP_FACEGEN)
			detour_jump((g_plugin->get_base() + 0x679B20), (uintptr_t)&can_use_preprocessing_head);
#else
			patch_mem_nop((g_plugin->get_base() + 0x679BB2), 0x27);
#endif
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// 984

			// Working buried function.
			*(uintptr_t*)&path_printf_facegen = g_plugin->get_base() + 0x601B50;

			auto offset = g_plugin->get_base() + 0x68B6F3;
			// Remove useless stuff.
			patch_mem_nop(offset, 0x1C);
			patch_mem_nop(offset + 0x23, 0x36);

			// mov rcx, r13
			// lea rdx, qword ptr ss:[rbp-0x40]
			// mov r8d, 0x104
			// mov r9d, edi
			// cmp r9d, 0x2
			// mov eax, 0x7
			// cmove r9d, eax
			patch_mem(offset + 0x2D, { 0x4C, 0x89, 0xE9, 0x48, 0x8D, 0x55, 0xC0, 0x41, 0xB8, 0x04, 0x01, 0x00, 0x00,
				0x41, 0x89, 0xF9, 0x41, 0x83, 0xF9, 0x02, 0xB8, 0x07, 0x00, 0x00, 0x00, 0x44, 0x0F, 0x44, 0xC8 });
			
			detour_call(offset + 0x4A, (uintptr_t)path_printf_facegen);	

#if defined(XC_NO_ORIG_SKIP_FACEGEN)
			detour_jump((g_plugin->get_base() + 0x68B900), (uintptr_t)&can_use_preprocessing_head);
#else
			patch_mem_nop((g_plugin->get_base() + 0x68B994), 0x27);
#endif
		}

		return true;
	}

	bool patch_facegen::can_use_preprocessing_head(TESNPC* npc_form)
	{
		if (!npc_form) return false;
		// list them until find the main form.
		while (npc_form->templateNPC)
			npc_form = npc_form->templateNPC;	
		// if the mod has set this option, i prohibit the use of preliminary data.
		if (npc_form->actorData.flags & TESActorBaseData::kFlagIsPreset)
			return false;
		// player form can't have a facegen.
		return npc_form->formID != 0x7;
	}
}