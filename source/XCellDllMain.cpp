// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/PluginAPI.h>

// XCell
#include <XCellVersion.h>
#include <XCellPlugin.h>

static XCell::Context* XCellContext = nullptr;

extern "C"
{
#if (FO4_VER < FO4_984_VERSION)
	// for f4se 6.23 and older
	__declspec(dllexport) bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info)
	{
		if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_163)
			return false;

		info->infoVersion = PluginInfo::kInfoVersion;
		info->version = XCell::ModVersion;
		info->name = XCell::ModName;

		return true;
	}
#else
	// for f4se 7.0 and newer
	__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version =
	{
		F4SEPluginVersionData::kVersion,
		XCell::ModVersion,
		MODNAME,
		AUTHOR,
		0,
		0,
		{ RUNTIME_VERSION_1_10_984, 0 },
		0,	// works with any version of the script extender. you probably do not need to put anything here
	};
#endif // !FO4_V984

	__declspec(dllexport) bool F4SEPlugin_Load(const F4SEInterface* f4se)
	{
		if (f4se->isEditor)
			return false;

		if (SUCCEEDED(XCell::CreateContextInstance(&XCellContext, f4se)))
		{
			if (FAILED(XCellContext->Initialize()))
			{
				XCell::ReleaseContextInstance(XCellContext);
				return false;
			}

			XCell::SetDefaultContextInstance(XCellContext);

			if (FAILED(XCellContext->Perform()))
			{
				XCell::ReleaseContextInstance(XCellContext);
				return false;
			}

			return true;
		}

		return false;
	}
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
    switch (dwReasonForCall)
    {
	case DLL_PROCESS_ATTACH:
		XCell::RegisterModule(hModule);
		break;
    case DLL_PROCESS_DETACH:
		if (SUCCEEDED(XCell::ReleaseContextInstance(XCellContext)))
			XCellContext = nullptr;
		XCell::UnregisterModule();
        break;
    }

    return TRUE;
}

