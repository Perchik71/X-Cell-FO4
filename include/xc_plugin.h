// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/PluginAPI.h>
#include <ms_rtti.h>
#include <xc_settings.h>
#include <xc_patch.h>
#include <vector>

namespace xc
{
	class plugin
	{
	public:
		plugin(const F4SEInterface* f4se);
		plugin(const plugin& p);

		plugin& operator=(const plugin& p);
		
		inline PluginHandle get_handle() const noexcept { return _handle; }
		inline UInt32 get_f4se_version() const noexcept { return _f4se_version; }
		inline UInt32 get_runtime_version() const noexcept { return _runtime_version; }
		inline settings* get_settings() noexcept { return &_settings; }
		inline uintptr_t get_base() const noexcept { return _base; }

		static void f4se_messages_handler(F4SEMessagingInterface::Message* msg);

		bool init();
		void output_info();
		void run();
	private:
		void send_massages_game_data_ready();
		bool get_pe_section_range(uintptr_t module_base, const char* section, uintptr_t* start, uintptr_t* end);

		PluginHandle _handle;
		UInt32 _f4se_version;
		UInt32 _runtime_version;
		settings _settings;
		vector<patch*> _patches;
		uintptr_t _base;
		msrtti::section _section[3];
		F4SEMessagingInterface* _messages;
		const F4SEInterface* _f4se;
	};
}

extern xc::plugin* g_plugin;