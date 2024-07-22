// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/PluginAPI.h>
#include <xc_version.h>
#include <xc_plugin.h>

xc::plugin* g_plugin = nullptr;

extern "C"
{
	// for f4se 7.0 and newer
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version =
	{
		F4SEPluginVersionData::kVersion,
        xc::modver,
        MODNAME,
        AUTHOR,
		0,
		0,
		{ RUNTIME_VERSION_1_10_984, 0 },
		0,	// works with any version of the script extender. you probably do not need to put anything here
	};

	// for f4se 6.23 and older
	__declspec(dllexport) bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info)
	{
		if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_163)
			return false;

		info->infoVersion = xc::modver;
		info->version = F4SEPlugin_Version.pluginVersion;
		info->name = xc::modname;

		return true;
	}

	__declspec(dllexport) bool F4SEPlugin_Load(const F4SEInterface* f4se)
	{
		if (f4se->isEditor)
			return false;
		
		g_plugin = new xc::plugin(f4se);
		if (g_plugin)
		{
			g_plugin->init();
			g_plugin->output_info();
			g_plugin->run();

			return true;
		}

		return false;
	}
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH:
		{
			HMODULE temp;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCSTR)hModule, &temp);
		}
		break;
    case DLL_PROCESS_DETACH:
		if (g_plugin)
		{
			delete g_plugin;
			g_plugin = nullptr;
		}
        break;
    }

    return TRUE;
}

