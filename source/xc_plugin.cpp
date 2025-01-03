﻿// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_plugin.h>
#include <xc_version.h>

// patches
#include <xc_patch_threads.h>
#include <xc_patch_memory.h>
#include <xc_patch_facegen.h>
#include <xc_patch_io.h>
#include <xc_patch_libdeflate.h>
#include <xc_patch_profile.h>
#include <xc_patch_archive_limit.h>
#include <xc_patch_loadscreen.h>

// fixes
#include <xc_fix_greymovies.h>
#include <xc_fix_packageallocatelocation.h>
#include <xc_fix_sampler.h>

namespace xc
{
	plugin::plugin(const F4SEInterface* f4se) : _messages(nullptr), _f4se(f4se)
	{
		_handle = f4se->GetPluginHandle();
		_f4se_version = f4se->f4seVersion;
		_runtime_version = f4se->runtimeVersion;
	}

	plugin::plugin(const plugin& p) : _handle(p._handle), _f4se_version(p._f4se_version),
		_runtime_version(p._runtime_version), _settings(p._settings), _messages(p._messages),
		_f4se(p._f4se)
	{}

	plugin& plugin::operator=(const plugin& p)
	{
		_handle = p._handle;
		_f4se_version = p._f4se_version;
		_runtime_version = p._runtime_version;
		_settings = p._settings;

		return *this;
	}

	string plugin::read_setting_str(const char* section, const char* name, const char* default_value) const noexcept
	{
		return _usersettings.has(section, name) ?
			_usersettings.read_str(section, name, default_value) :
			_settings.read_str(section, name, default_value);
	}

	int32_t plugin::read_setting_int(const char* section, const char* name, int32_t default_value) const noexcept
	{
		return _usersettings.has(section, name) ?
			_usersettings.read_int(section, name, default_value) :
			_settings.read_int(section, name, default_value);
	}

	uint32_t plugin::read_setting_uint(const char* section, const char* name, uint32_t default_value) const noexcept
	{
		return _usersettings.has(section, name) ?
			_usersettings.read_uint(section, name, default_value) :
			_settings.read_uint(section, name, default_value);
	}

	float plugin::read_setting_float(const char* section, const char* name, float default_value) const noexcept
	{
		return _usersettings.has(section, name) ?
			_usersettings.read_float(section, name, default_value) :
			_settings.read_float(section, name, default_value);
	}

	bool plugin::read_setting_bool(const char* section, const char* name, bool default_value) const noexcept
	{
		return _usersettings.has(section, name) ?
			_usersettings.read_bool(section, name, default_value) :
			_settings.read_bool(section, name, default_value);
	}

	void plugin::write_setting_str(const char* section, const char* name, const char* value) const noexcept
	{
		_usersettings.write_str(section, name, value);
	}

	void plugin::write_setting_int(const char* section, const char* name, int32_t value) const noexcept
	{
		_usersettings.write_int(section, name, value);
	}

	void plugin::write_setting_uint(const char* section, const char* name, uint32_t value) const noexcept
	{
		_usersettings.write_uint(section, name, value);
	}

	void plugin::write_setting_float(const char* section, const char* name, float value) const noexcept
	{
		_usersettings.write_float(section, name, value);
	}

	void plugin::write_setting_bool(const char* section, const char* name, bool value) const noexcept
	{
		_usersettings.write_bool(section, name, value);
	}

	void plugin::f4se_messages_handler(F4SEMessagingInterface::Message* msg)
	{
		if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady)
		{
			_MESSAGE("The game passed the data");
			g_plugin->send_massages_game_data_ready();
		}
	}

	bool plugin::register_funcs_vm(VirtualMachine* vm)
	{
		_MESSAGE("The game passed the register papyrus functions");
		fix_sampler::register_functions(vm);

		return true;
	}

	bool plugin::init()
	{
		gLog.OpenRelative(5, "\\My Games\\Fallout4\\F4SE\\" MODNAME ".log");
		
		_base = (uintptr_t)GetModuleHandleA(NULL);

		char szBuffer[MAX_PATH];
		GetModuleFileNameA((HMODULE)_base, szBuffer, MAX_PATH);
		szBuffer[MAX_PATH - 1] = '\0';

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			_ERROR("The path to app is too large, more than MAX_PATH (%u)", MAX_PATH);
			return false;
		}

		_usersettings.set_filename(5, "\\My Games\\Fallout4\\F4SE\\" MODNAME ".ini");

		string path = szBuffer;
		auto si = path.find_last_of("\\/");
		if (si == string::npos)
		{
			_ERROR("The path to app is strange \"%s\"", path.c_str());
			return false;
		}

		if (!get_pe_section_range(_base, ".text", &_section[0].base, &_section[0].end))
		{
			_ERROR("There is no information about \".text\" in the module", path.c_str());
			return false;
		}

		if (!get_pe_section_range(_base, ".rdata", &_section[1].base, &_section[1].end))
		{
			_ERROR("There is no information about \".rdata\" in the module", path.c_str());
			return false;
		}

		if (!get_pe_section_range(_base, ".data", &_section[2].base, &_section[2].end))
		{
			_ERROR("There is no information about \".data\" in the module", path.c_str());
			return false;
		}

		uintptr_t tempStart, tempEnd;
		if (get_pe_section_range(_base, ".textbss", &tempStart, &tempEnd))
		{
			_section[0].base = std::min(_section[0].base, tempStart);
			_section[0].end = std::max(_section[0].end, tempEnd);
		}

		if (get_pe_section_range(_base, ".interpr", &tempStart, &tempEnd))
		{
			_section[0].base = std::min(_section[0].base, tempStart);
			_section[0].end = std::max(_section[0].end, tempEnd);
		}

		msrtti::init(_base, _section[0], _section[2], _section[1]);

		path = path.substr(0, si);
		if (_settings.set_filename((path + "\\Data\\F4SE\\Plugins\\" MODNAME ".ini").c_str()))
			_MESSAGE("The settings file has been loaded successfully");

		if (_settings.read_bool("additional", "rtti_output", false))
		{
			auto rtti_fname = path + "\\Data\\F4SE\\Plugins\\rtti-" MODNAME ".txt";
			FILE* f = _fsopen(rtti_fname.c_str(), "wb", _SH_DENYWR);
			if (!f)
				_MESSAGE("Failed to create a \"%s\" file for RTTI output", rtti_fname.c_str());
			else
			{
				msrtti::dump(f);
				fclose(f);
			}
		}

		// get the serialization interface and query its version
		if (!(_messages = (F4SEMessagingInterface*)_f4se->QueryInterface(kInterface_Messaging)))
		{
			_FATALERROR("Couldn't get messaging interface");
			return false;
		}
		else
		{
			if (_messages->interfaceVersion < F4SEMessagingInterface::kInterfaceVersion)
			{
				_FATALERROR("F4SEMessagingInterface interface too old (%d expected %d)",
					_messages->interfaceVersion, F4SEMessagingInterface::kInterfaceVersion);
				return false;
			}

			if (!_messages->RegisterListener(_handle, "F4SE", f4se_messages_handler))
			{
				_FATALERROR("Failed to register a message handler");
				return false;
			}
		}

		_papyrus = (F4SEPapyrusInterface*)_f4se->QueryInterface(kInterface_Papyrus);
		if (!_papyrus)
		{
			_WARNING("couldn't get papyrus interface");
			return false;
		}
		else
		{
			if (_papyrus->interfaceVersion < F4SEPapyrusInterface::kInterfaceVersion)
			{
				_FATALERROR("F4SEPapyrusInterface interface too old (%d expected %d)",
					_papyrus->interfaceVersion, F4SEPapyrusInterface::kInterfaceVersion);
				return false;
			}

			if (!_papyrus->Register(register_funcs_vm))
			{
				_FATALERROR("Failed to register a message handler");
				return false;
			}
		}

		//////////////////

		_patches.push_back(new patch_threads());
		_patches.push_back(new patch_memory());
		_patches.push_back(new patch_facegen());
		_patches.push_back(new patch_io());
		_patches.push_back(new patch_libdeflate());
		_patches.push_back(new patch_profile());
		_patches.push_back(new patch_archive_limit());
		_patches.push_back(new patch_loadscreen());

		//////////////////

		_fixes.push_back(new fix_greymovies());
		_fixes.push_back(new fix_package_allocate_location());
		_fixes.push_back(new fix_sampler());

		//////////////////

		return true;
	}

	void plugin::output_info()
	{
		_MESSAGE("F4SE version check. f4seVersion: 0x%x, runtimeVersion: 0x%x", _f4se_version, _runtime_version);
		_MESSAGE("Plugin \"" MODNAME "\" version check. Version: 0x%x, Author: %s", xc::modver, xc::author);
		_MESSAGE("The base: %llX", _base);
		_MESSAGE("The \".text\" section: (base: %llX end: %llX)", _section[0].base, _section[0].end);
		_MESSAGE("The \".rdata\" section: (base: %llX end: %llX)", _section[1].base, _section[1].end);
		_MESSAGE("The \".data\" section: (base: %llX end: %llX)", _section[2].base, _section[2].end);
	}

	void plugin::run()
	{
		__try
		{
			for (auto i : _patches)
				if (i) i->start("patches");

			for (auto i : _fixes)
				if (i) i->start("fixes");
		}
		__except (1)
		{
			_FATALERROR("Error: Errors occurred during patch initialization");
		}
	}

	void plugin::send_massages_game_data_ready()
	{
		__try
		{
			for (auto i : _patches)
				if (i) i->game_data_ready_handler();

			for (auto i : _fixes)
				if (i) i->game_data_ready_handler();
		}
		__except (1)
		{
			_ERROR("Error: Sending the message \"Game Data Ready\" failed");
		}
	}

	bool plugin::get_pe_section_range(uintptr_t module_base, const char* section, uintptr_t* start, uintptr_t* end)
	{
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(module_base + ((PIMAGE_DOS_HEADER)module_base)->e_lfanew);
		PIMAGE_SECTION_HEADER cur_section = IMAGE_FIRST_SECTION(ntHeaders);

		if (!section || strlen(section) <= 0)
		{
			if (start)
				*start = module_base;

			if (end)
				*end = module_base + ntHeaders->OptionalHeader.SizeOfHeaders;

			return true;
		}

		for (uint32_t i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, cur_section++)
		{
			char sectionName[sizeof(IMAGE_SECTION_HEADER::Name) + 1] = { };
			memcpy(sectionName, cur_section->Name, sizeof(IMAGE_SECTION_HEADER::Name));

			if (!strcmp(sectionName, section))
			{
				if (start)
					*start = module_base + cur_section->VirtualAddress;

				if (end)
					*end = module_base + cur_section->VirtualAddress + cur_section->Misc.VirtualSize;

				return true;
			}
		}

		return false;
	}
}