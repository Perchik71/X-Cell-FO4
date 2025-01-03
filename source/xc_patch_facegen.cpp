﻿// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#define XC_NO_ORIG_SKIP_FACEGEN

#include <mini/ini.h>
#include <f4se/GameObjects.h>
#include <f4se/GameForms.h>
#include <f4se/GameData.h>
#include <xc_patch_facegen.h>
#include <xc_version.h>
#include <xc_plugin.h>
#include <vector>

// take from f4se 6.23
// 856197F11173AF60E35EBF54A88E7BF43AFC3588+305
RelocPtr<DataHandler*> g_dataHandler_OG(0x058CF080);

namespace xc
{
	inline static const string& trim(const string& str)
	{
		static const char* whitespaceDelimiters = " \t\n\r\f\v";

		const_cast<string&>(str).erase(str.find_last_not_of(whitespaceDelimiters) + 1);
		const_cast<string&>(str).erase(0, str.find_first_not_of(whitespaceDelimiters));

		return str;
	}

	BGSKeyword** g_keyword_is_child_player = nullptr;
	vector<UInt32> g_facegen_primary_exception_formids =
	{
		// Since it has no protection in the game

		0x26F0A,	// MQ102PlayerSpouseCorpseMale
		0x26F36,	// MQ102PlayerSpouseCorpseFemale
		0xA7D34,	// MQ101PlayerSpouseMale
		0xA7D35,	// MQ101PlayerSpouseFemale
		0x246BF0,	// MQ101PlayerSpouseMale_NameOnly
		0x246BF1,	// MQ101PlayerSpouseFemale_NameOnly
	};
	vector<UInt32> g_facegen_exception_formids;
	DataHandler** dataHandler = nullptr;

	enum TESActorFlags_XCELL
	{
		kFlagSimpleActor = 0x100000
	};

	static bool detect_load_order_formid(const string& plugin_name, UInt32& formid)
	{
		constexpr static UInt16 INVALID_INDEX = (UInt16)-1;

		__try
		{
			UInt16 index_plugin = INVALID_INDEX;

			if (!plugin_name.empty())
			{
				// Search among master, default plugins
				if ((index_plugin = (*dataHandler)->GetLoadedModIndex(plugin_name.c_str())) != INVALID_INDEX)
					formid = (formid & (0x00FFFFFF)) | (index_plugin << 24);
				// Search among light master plugins
				else if ((index_plugin = (*dataHandler)->GetLoadedLightModIndex(plugin_name.c_str())) != INVALID_INDEX)
					formid = (formid & (0x00000FFF)) | (index_plugin << 12) | 0xFE000000;
				// If there is no such thing, then it is a waste of a stupid user's time
				else
				{
					_ERROR("Failed NPC added (no found plugin) \"%s\" (%08X)", plugin_name, formid);
					return false;
				}
			}
			
			return true;
		}
		__except (1)
		{
			_ERROR("Failed NPC added (fatal error) \"%s\" (%08X)", plugin_name, formid);
			return false;
		}
	}

	const char* patch_facegen::get_name() const noexcept
	{
		return "facegen";
	}

	bool patch_facegen::game_data_ready_handler() const noexcept
	{
		g_facegen_exception_formids = g_facegen_primary_exception_formids;
		// Read the file again to get faster access to the desired section
		mINI::INIFile settings_file(g_plugin->get_settings()->get_filename());
		mINI::INIStructure settings_data;
		if (settings_file.read(settings_data) && settings_data.has("facegen_exception"))
		{
			char* end_ptr = nullptr;
			UInt32 formid = 0;
			auto section = settings_data.get("facegen_exception");

			for (auto it = section.begin(); it != section.end(); it++)
			{
				if (it->second.empty()) continue;

				std::string value, plugin_name;
				auto it_sep = it->second.find_first_of(':');
				if (it_sep != std::string::npos)
				{
					plugin_name = trim(it->second.substr(it_sep + 1));
					value = trim(it->second.substr(0, it_sep));
				}
				else
					value = trim(it->second);

				if (value.find_first_of("0x") == 0)
					formid = strtoul(it->second.c_str() + 2, &end_ptr, 16);
				else
					formid = strtoul(it->second.c_str(), &end_ptr, 10);

				if (detect_load_order_formid(plugin_name, formid))
				{
					_MESSAGE("Skip NPC added \"%s\" (%08X)", it->first.c_str(), formid);
					g_facegen_exception_formids.push_back(formid);					
				}
			}
		}

		return true;
	}

	bool patch_facegen::run() const
	{
		// The problem is, the game uses facegen, which lies only in fallout4.esm.

		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// 163
			dataHandler = g_dataHandler_OG.GetPtr();

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

			// added for check player npc
			g_keyword_is_child_player = (BGSKeyword**)(g_plugin->get_base() + 0x59DADD0);
#if defined(XC_NO_ORIG_SKIP_FACEGEN)
			detour_jump((g_plugin->get_base() + 0x679B20), (uintptr_t)&can_use_preprocessing_head);
#else
			patch_mem_nop((g_plugin->get_base() + 0x679BB2), 0x27);
#endif
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// 984
			dataHandler = g_dataHandler.GetPtr();

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

			// added for check player npc
			g_keyword_is_child_player = (BGSKeyword**)(g_plugin->get_base() + 0x2F6ED50);
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
		if ((npc_form->actorData.flags & TESActorBaseData::kFlagIsPreset) ||
			(npc_form->actorData.flags & TESActorFlags_XCELL::kFlagSimpleActor))
			return false;
		// check if the NPC is a relative or a template for the player.
		auto size = npc_form->keywords.numKeywords;
		for (size_t i = 0; i < size; i++)
			if (npc_form->keywords.keywords[i] == *g_keyword_is_child_player)
				return false;
		// optionally exclude some NPCs.
		for (auto it_except : g_facegen_exception_formids)
			if (npc_form->formID == it_except) return false;
		// player form can't have a facegen.
		return npc_form->formID != 0x7;
	}
}