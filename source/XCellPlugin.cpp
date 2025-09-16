// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <ShlObj_core.h>

// XCell
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellTableID.h"
#include "XCellStringUtils.h"
#include "XCellCommonType.h"

// XCell modules
#include "XCellModuleControlSamples.h"
#include "XCellModuleImGUI.h"
#include "XCellModuleThreads.h"
#include "XCellModuleMemory.h"
#include "XCellModuleIO.h"
#include "XCellModuleFacegen.h"
#include "XCellModuleLibDeflate.h"
#include "XCellModuleProfile.h"
#include "XCellModuleLoadScreen.h"
#include "XCellModuleUpscaler.h"
#include "XCellModuleGreyMovies.h"
#include "XCellModulePackageAllocateLocation.h"
#include "XCellModuleWarningCreateTexture2D.h"
#include "XCellModuleInitTints.h"
#include "XCellModuleLODDistanceFix.h"
#include "XCellModuleDropItem.h"

#define XCELL_IMGUI_INSTALL 0

namespace XCell
{
	Context* gContext = nullptr;
	HMODULE gModuleHandle = nullptr;
	DirectXData* gDirectXData = nullptr;
	UInt64 gOldDXGIPresentFunctions = 0;
	UInt64 gOldInterface3D_Renderer_RenderPostAA = 0;

	static void XCListenerDX11RenderEndFrame(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
	{
		if (gContext && (Flags != DXGI_PRESENT_TEST))
			gContext->Listener(Event::EventRenderEndFrame);

		XCFastCall<void>(gOldDXGIPresentFunctions, This, SyncInterval, Flags);
	}

	static void XCListener_Interface3D_Renderer_RenderPostAA(UInt32 Unk01, bool Unk02)
	{
		if (gContext)
		{
			gContext->Listener(Event::EventPrepareUIDrawCuled);
			XCFastCall<void>(gOldInterface3D_Renderer_RenderPostAA, Unk01, Unk02);
		}
	}

	static void XCListenerDX11()
	{
		if (gContext && gDirectXData)
		{
			gContext->Listener(Event::EventInitializeDirectX, gDirectXData->window, gDirectXData->device, gDirectXData->context,
				gDirectXData->swap_chain);

			gOldDXGIPresentFunctions = REL::Impl::DetourVTable(*((UInt64*)gDirectXData->swap_chain), 
				(UInt64)&XCListenerDX11RenderEndFrame, 8);
		}
		// continue...
		XCFastCall<void>(REL::ID(20));
	}

	HRESULT Context::InitializeLogger() const noexcept(true)
	{
		const uint32_t BufferSize = 1024;
		auto Buffer = make_unique<char[]>((size_t)BufferSize + 1);
		if (!Buffer)
		{
		UnknownDllName:
			gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\UnknownPluginName.log");
		}
		else
		{
			if (!GetModuleFileNameA(gModuleHandle, Buffer.get(), BufferSize))
				goto UnknownDllName;

			const auto FormattedString = "\\My Games\\Fallout4\\F4SE\\%s.log";

			PathStripPathA(Buffer.get());
			auto FileName = Buffer.get();
			auto ExtPosition = strchr(FileName, '.');
			if (ExtPosition)
				*ExtPosition = 0;

			auto NeedLen = _scprintf(FormattedString, FileName);
			if (NeedLen <= 0)
				goto UnknownDllName;

			auto Path = make_unique<char[]>((size_t)NeedLen + 1);
			if (!Path)
				goto UnknownDllName;

			sprintf(Path.get(), FormattedString, Buffer.get());
			gLog.OpenRelative(CSIDL_MYDOCUMENTS, Path.get());

			//_MESSAGE("[DEBUG] FileName:\"%s\" Path:\"%s\"",  FileName, Path.get());
		}

		return S_OK;
	}

	HRESULT Context::InitializeCommon() noexcept(true)
	{
		_MESSAGE("F4SE version check. f4seVersion: 0x%x, runtimeVersion: 0x%x", _f4se_interface->f4seVersion, _f4se_interface->runtimeVersion);
		_MESSAGE("Plugin \"" MODNAME "\" version check. Version: 0x%x, Author: %s", XCell::ModVersion, XCell::Author);

		_app_path = Utils::GetApplicationPath();

		_MESSAGE("AppPath: %s", _app_path.c_str());

		return S_OK;
	}

	HRESULT Context::InitializeCVar() noexcept(true)
	{
		HRESULT Result = SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, Game::gINIGameSettingPref);
		strcat_s(Game::gINIGameSettingPref, "\\My Games\\Fallout4\\Fallout4Prefs.ini");

		// Patches
		_settings.Add(CVarThreads);
		_settings.Add(CVarMemory);
		_settings.Add(CVarFacegen);
		_settings.Add(CVarIO);
		_settings.Add(CVarLibDeflate);
		_settings.Add(CVarProfile);
		_settings.Add(CVarLoadScreen);
		_settings.Add(CVarUpscaler);

		// Fixes
		_settings.Add(CVarInitTints);
		_settings.Add(CVarLODDistance);
		_settings.Add(CVarGreyMovies);
		_settings.Add(CVarDropItem);
		_settings.Add(CVarPackageAllocateLocation);
		_settings.Add(CVarWarningCreateTexture2D);

		// Additional
		_settings.Add(CVarScaleformPageSize);
		_settings.Add(CVarScaleformHeapSize);
		_settings.Add(CVarUseNewRedistributable);
		_settings.Add(CVarOutputRTTI);
		_settings.Add(CVarUseIORandomAccess);
		_settings.Add(CVarDbgFacegenOutput);

		// Graphics
		_settings.Add(CVarDisplayScale);
		_settings.Add(CVarNoUseTAA);

		// Load settings
		if (!_settings.LoadFromFile((_app_path + "Data\\F4SE\\Plugins\\x-cell.toml").c_str()))
			return E_FAIL;

		CVarDisplayScale->SetFloat(max(0.5f, min(1.0f, CVarDisplayScale->GetFloat())));

		return S_OK;
	}

	HRESULT Context::InitializeRTTI() noexcept(true)
	{
		_base = (UInt64)GetModuleHandleA(nullptr);

		if (!GetPESectionRange(".text", &_section[0]))
		{
			_ERROR("There is no information about \".text\" in the module");
			return E_FAIL;
		}

		if (!GetPESectionRange(".rdata", &_section[1]))
		{
			_ERROR("There is no information about \".rdata\" in the module");
			return E_FAIL;
		}

		if (!GetPESectionRange(".data", &_section[2]))
		{
			_ERROR("There is no information about \".data\" in the module");
			return E_FAIL;
		}

		msrtti::section temp;
		if (GetPESectionRange(".textbss", &temp))
		{
			_section[0].base = min(_section[0].base, temp.base);
			_section[0].end = max(_section[0].end, temp.end);
		}

		if (GetPESectionRange(".interpr", &temp))
		{
			_section[0].base = min(_section[0].base, temp.base);
			_section[0].end = max(_section[0].end, temp.end);
		}

		msrtti::init(_base, _section[0], _section[2], _section[1]);

		_MESSAGE("The base: %llX", _base);
		_MESSAGE("The \".text\" section: (base: %llX end: %llX)", _section[0].base, _section[0].end);
		_MESSAGE("The \".rdata\" section: (base: %llX end: %llX)", _section[1].base, _section[1].end);
		_MESSAGE("The \".data\" section: (base: %llX end: %llX)", _section[2].base, _section[2].end);

		// Output RTTI
		// WARNING: InitializeCVar() call before InitializeRTTI()
		if (CVarOutputRTTI->GetBool())
		{
			auto rtti_fname = _app_path + "\\Data\\F4SE\\Plugins\\rtti-" MODNAME ".txt";
			FILE* f = _fsopen(rtti_fname.c_str(), "wb", _SH_DENYWR);
			if (!f)
				_MESSAGE("Failed to create a \"%s\" file for RTTI output.", rtti_fname.c_str());
			else
			{
				msrtti::dump(f);
				fclose(f);
			}
		}
			
		return S_OK;
	}

	HRESULT Context::InitializeGraphicPending() noexcept(true)
	{
		// maybe NiRenderWindow
		gDirectXData = (DirectXData*)REL::ID(10);
		_graphics_listener.Install(REL::ID(0), (UInt64)XCListenerDX11);
		REL::Impl::DetourCall(REL::ID(22), (UInt64)XCListener_Interface3D_Renderer_RenderPostAA);
		gOldInterface3D_Renderer_RenderPostAA = REL::ID(21);
		if (FAILED(_graphics_listener.Enable())) return E_FAIL;
		return S_OK;
	}

	HRESULT Context::InitializePapyrusPending() noexcept(true)
	{
		if (!_f4se_interface)
			return E_FAIL;

		if (!(_papyrus = (F4SEPapyrusInterface*)_f4se_interface->QueryInterface(kInterface_Papyrus)))
		{
			_WARNING("couldn't get papyrus interface");
			return E_FAIL;
		}
		else
		{
			if (_papyrus->interfaceVersion < F4SEPapyrusInterface::kInterfaceVersion)
			{
				_FATALERROR("F4SEPapyrusInterface interface too old (%d expected %d)",
					_papyrus->interfaceVersion, F4SEPapyrusInterface::kInterfaceVersion);
				return E_FAIL;
			}

			if (!_papyrus->Register([](VirtualMachine* vm) -> bool {
				if (gContext) gContext->Listener(Event::EventInitializePapyrus, vm);
				return true;
				}))
			{
				_FATALERROR("Failed to register a message handler");
				return E_FAIL;
			}
		}

		return S_OK;
	}

	HRESULT Context::InitializeMessagePending() noexcept(true)
	{
		if (!_f4se_interface)
			return E_FAIL;

		// get the serialization interface and query its version
		if (!(_messages = (F4SEMessagingInterface*)_f4se_interface->QueryInterface(kInterface_Messaging)))
		{
			_FATALERROR("Couldn't get messaging interface");
			return E_FAIL;
		}
		else
		{
			if (_messages->interfaceVersion < F4SEMessagingInterface::kInterfaceVersion)
			{
				_FATALERROR("F4SEMessagingInterface interface too old (%d expected %d)",
					_messages->interfaceVersion, F4SEMessagingInterface::kInterfaceVersion);
				return E_FAIL;
			}

			if (!_messages->RegisterListener(_f4se_interface->GetPluginHandle(), "F4SE", [](F4SEMessagingInterface::Message* msg) {
				// sent when the data handler is ready (data is false before loading, true when finished loading)
				if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady)
				{
					_MESSAGE("The game passed the data");
					if (gContext) gContext->Listener(Event::EventGameDataReady);
				} 
				// sent after the game has finished loading (only sent once)
				else if (msg->type == F4SEMessagingInterface::kMessage_GameLoaded)
				{
					_MESSAGE("The game loaded");
					if (gContext) gContext->Listener(Event::EventGameLoaded);
				} 
				// sent after a new game is created, before the game has loaded (Sends CharGen TESQuest pointer)
				else if (msg->type == F4SEMessagingInterface::kMessage_NewGame)
				{
					_MESSAGE("The game passed the start new game");
					if (gContext) gContext->Listener(Event::EventNewGame);
				}
				}))
			{
				_FATALERROR("Failed to register a message handler");
				return E_FAIL;
			}
		}

		return S_OK;
	}

	HRESULT Context::InitializeModules() noexcept(true)
	{
		if (FAILED(_modules.Add(make_shared<ModuleControlSamples>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleFacegen>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleUpscaler>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleThreads>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleMemory>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleIO>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleLibDeflate>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleProfile>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleLoadScreen>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleGreyMovies>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModulePackageAllocateLocation>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleWarningCreateTexture2D>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleInitTints>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleLODDistanceFix>(this)))) return E_FAIL;
		if (FAILED(_modules.Add(make_shared<ModuleDropItem>(this)))) return E_FAIL;		
		// Required install after all modules
#if XCELL_IMGUI_INSTALL
		if (FAILED(_modules.Add(make_shared<ModuleImGUI>(this)))) return E_FAIL;
#endif // XCELL_IMGUI_INSTALL

		return S_OK;
	}

	bool Context::GetPESectionRange(const char* section, msrtti::section* data) const noexcept(true)
	{
		if (!data)
			return false;

		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(_base + ((PIMAGE_DOS_HEADER)_base)->e_lfanew);
		PIMAGE_SECTION_HEADER cur_section = IMAGE_FIRST_SECTION(ntHeaders);

		if (!section || (strlen(section) <= 0))
		{
			data->base = _base;
			data->end = data->base + ntHeaders->OptionalHeader.SizeOfHeaders;

			return true;
		}

		for (uint32_t i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, cur_section++)
		{
			char sectionName[sizeof(IMAGE_SECTION_HEADER::Name) + 1] = { };
			memcpy(sectionName, cur_section->Name, sizeof(IMAGE_SECTION_HEADER::Name));

			if (!strcmp(sectionName, section))
			{
				data->base = _base + cur_section->VirtualAddress;
				data->end = data->base + cur_section->Misc.VirtualSize;

				return true;
			}
		}

		return false;
	}

	HRESULT Context::PerformImpl() noexcept(true)
	{
		for (UInt64 Index = 0; Index < _modules.Size(); Index++)
		{
			auto Mod = _modules[Index];
			auto hr = Mod->Install();
			if (FAILED(hr))
			{
				_FATALERROR("Module \"%s\" initialization error, it is unsafe to continue.", Mod->Name.c_str());
				return E_FAIL;
			}
			else if (hr == S_OK)
				_MESSAGE("Module \"%s\" has been successfully initialized.", Mod->Name.c_str());
		}

		return S_OK;
	}

	HRESULT Context::ShutdownImpl() noexcept(true)
	{
		_graphics_listener.Disable();

		for (UInt64 Index = 0; Index < _modules.Size(); Index++)
		{
			auto Mod = _modules[Index];
			auto hr = Mod->Shutdown();
			if (FAILED(hr))
			{
				_FATALERROR("Module \"%s\" release error, it is unsafe to continue.", Mod->Name.c_str());
				return E_FAIL;
			}
			else if (hr == S_OK)
				_MESSAGE("Module \"%s\" has been successfully released.", Mod->Name.c_str());
		}

		return S_OK;
	}

	Context::Context(const F4SEInterface* f4se) :
		_f4se_interface(f4se)
	{}

	Context::~Context()
	{
		Shutdown();
	}

	HRESULT Context::Initialize() noexcept(true)
	{
		if (!_f4se_interface)
			return E_FAIL;

		if (_f4se_interface->runtimeVersion == RUNTIME_VERSION_1_10_163)
			REL::K_CUR = &REL::K_163;
		else if (_f4se_interface->runtimeVersion == RUNTIME_VERSION_1_10_984)
			REL::K_CUR = &REL::K_984;
		else
			return E_FAIL;

		if (FAILED(InitializeLogger()))
			return E_FAIL;

		if (FAILED(InitializeCommon()))
			return E_FAIL;

		if (FAILED(InitializeCVar()))
			return E_FAIL;

		if (FAILED(InitializeRTTI()))
			return E_FAIL;

		if (FAILED(InitializeGraphicPending()))
			return E_FAIL;

		if (FAILED(InitializePapyrusPending()))
			return E_FAIL;

		if (FAILED(InitializeMessagePending()))
			return E_FAIL;

		if (FAILED(InitializeModules()))
			return E_FAIL;

		return S_OK;
	}

	HRESULT Context::Perform() noexcept(true)
	{
		__try
		{
			_modules.Lock();
			auto Result = PerformImpl();
			_modules.Unlock();
			return Result;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			_modules.Unlock();
			_FATALERROR("When initializing the models, something irreparable happened.");
			return E_FAIL;
		}
	}

	HRESULT Context::Shutdown() noexcept(true)
	{
		__try
		{
			_modules.Lock();
			auto Result = ShutdownImpl();
			_modules.Unlock();
			return Result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			_modules.Unlock();
			_FATALERROR("When release the models, something irreparable happened.");
			return E_FAIL;
		}

		return S_OK;
	}

	void Context::Listener(Event::ListenerEventType Type, ...)
	{
		if (Type == Event::EventInitializeDirectX)
		{
			va_list ap;
			va_start(ap, &Type);

			auto WindowHandle = va_arg(ap, HWND);
			auto Device = va_arg(ap, ID3D11Device*);
			auto DeviceContext = va_arg(ap, ID3D11DeviceContext*);
			auto SwapChain = va_arg(ap, IDXGISwapChain*);

			_registered_initdx11_link.Listener(WindowHandle, Device, DeviceContext, SwapChain);
		}
		else if (Type == Event::EventInitializePapyrus)
		{
			va_list ap;
			va_start(ap, &Type);

			auto VM = va_arg(ap, VirtualMachine*);

			_registered_initvm_link.Listener(VM);
		}
		else if (Type == Event::EventGameDataReady)
			_registered_dataready_link.Listener();
		else if (Type == Event::EventGameLoaded)
			_registered_loaded_link.Listener();
		else if (Type == Event::EventNewGame)
			_registered_newgame_link.Listener();
		else if (Type == Event::EventRenderEndFrame)
			_registered_endframe_link.Listener();
		else if (Type == Event::EventPrepareUIDrawCuled)
			_registered_uidrawculed_link.Listener();	
	}

	void Context::RegisterListeners(Module* Module, UInt32 Listeners)
	{
		if (!Listeners || !Module)
			return;

		auto Mod = _modules.FindByName(Module->Name.c_str());
		if (!Mod) return;

		if ((Listeners & XCELL_MODULE_QUERY_DIRECTX_INIT) == XCELL_MODULE_QUERY_DIRECTX_INIT) 
			_registered_initdx11_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_PAPYRUS_INIT) == XCELL_MODULE_QUERY_PAPYRUS_INIT)
			_registered_initvm_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_DATA_READY) == XCELL_MODULE_QUERY_DATA_READY)
			_registered_dataready_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_GAME_LOADED) == XCELL_MODULE_QUERY_GAME_LOADED)
			_registered_loaded_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_NEW_GAME) == XCELL_MODULE_QUERY_NEW_GAME)
			_registered_newgame_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_END_FRAME) == XCELL_MODULE_QUERY_END_FRAME)
			_registered_endframe_link.RegisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_PREPARE_UI_DRAW) == XCELL_MODULE_QUERY_PREPARE_UI_DRAW)
			_registered_uidrawculed_link.RegisterModule(Mod);
	}

	void Context::UnregisterListeners(Module* Module, UInt32 Listeners)
	{
		if (!Listeners || !Module)
			return;

		auto Mod = _modules.FindByName(Module->Name.c_str());
		if (!Mod) return;

		if ((Listeners & XCELL_MODULE_QUERY_DIRECTX_INIT) == XCELL_MODULE_QUERY_DIRECTX_INIT)
			_registered_initdx11_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_PAPYRUS_INIT) == XCELL_MODULE_QUERY_PAPYRUS_INIT)
			_registered_initvm_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_DATA_READY) == XCELL_MODULE_QUERY_DATA_READY)
			_registered_dataready_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_GAME_LOADED) == XCELL_MODULE_QUERY_GAME_LOADED)
			_registered_loaded_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_NEW_GAME) == XCELL_MODULE_QUERY_NEW_GAME)
			_registered_newgame_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_END_FRAME) == XCELL_MODULE_QUERY_END_FRAME)
			_registered_endframe_link.UnregisterModule(Mod);
		if ((Listeners & XCELL_MODULE_QUERY_PREPARE_UI_DRAW) == XCELL_MODULE_QUERY_PREPARE_UI_DRAW)
			_registered_uidrawculed_link.UnregisterModule(Mod);
	}

	HRESULT CreateContextInstance(Context** pContext, const F4SEInterface* f4se) noexcept(true)
	{
		if (!pContext || !f4se)
			return OSS_BAD_ARG;

		*pContext = new Context(f4se);
		if (!(*pContext))
			return OSS_OUT_MEMORY;

		return S_OK;
	}

	HRESULT __stdcall ReleaseContextInstance(const Context* pContext) noexcept(true)
	{
		if (!pContext)
			return OSS_BAD_ARG;

		delete pContext;
		return S_OK;
	}

	HRESULT __stdcall GetDefaultContextInstance(Context** pContext) noexcept(true)
	{
		if (!pContext)
			return OSS_BAD_ARG;

		*pContext = gContext;
		return S_OK;
	}

	HRESULT __stdcall SetDefaultContextInstance(const Context* pContext) noexcept(true)
	{
		if (!pContext)
			return OSS_BAD_ARG;

		gContext = const_cast<Context*>(pContext);
		return S_OK;
	}

	HRESULT __stdcall RegisterModule(HMODULE hModule) noexcept(true)
	{
		HMODULE temp;
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			(LPCSTR)hModule, &temp))
		{
			gModuleHandle = hModule;
			return S_OK;
		}

		return S_FALSE;
	}

	HRESULT __stdcall UnregisterModule() noexcept(true)
	{
		gModuleHandle = nullptr;
		return S_OK;
	}
}