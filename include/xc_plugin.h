﻿// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/PluginAPI.h>
#include <f4se/PapyrusVM.h>
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
		inline settings* get_usersettings() noexcept { return &_usersettings; }
		inline uintptr_t get_base() const noexcept { return _base; }
		inline msrtti::section get_section(size_t index) const noexcept { return _section[index]; }

		string read_setting_str(const char* section, const char* name, const char* default_value) const noexcept;
		int32_t read_setting_int(const char* section, const char* name, int32_t default_value) const noexcept;
		uint32_t read_setting_uint(const char* section, const char* name, uint32_t default_value) const noexcept;
		float read_setting_float(const char* section, const char* name, float default_value) const noexcept;
		bool read_setting_bool(const char* section, const char* name, bool default_value) const noexcept;

		void write_setting_str(const char* section, const char* name, const char* value) const noexcept;
		void write_setting_int(const char* section, const char* name, int32_t value) const noexcept;
		void write_setting_uint(const char* section, const char* name, uint32_t value) const noexcept;
		void write_setting_float(const char* section, const char* name, float value) const noexcept;
		void write_setting_bool(const char* section, const char* name, bool value) const noexcept;


		static void f4se_messages_handler(F4SEMessagingInterface::Message* msg);

		bool init();
		void output_info();
		void run();

		static bool register_funcs_vm(VirtualMachine* vm);
	private:
		void send_massages_game_data_ready();
		bool get_pe_section_range(uintptr_t module_base, const char* section, uintptr_t* start, uintptr_t* end);

		PluginHandle _handle;
		UInt32 _f4se_version;
		UInt32 _runtime_version;
		settings _settings;
		settings _usersettings;
		vector<patch*> _patches;
		vector<patch*> _fixes;
		uintptr_t _base;
		msrtti::section _section[3];
		F4SEMessagingInterface* _messages;
		F4SEPapyrusInterface* _papyrus;
		const F4SEInterface* _f4se;
	};
}

extern xc::plugin* g_plugin;