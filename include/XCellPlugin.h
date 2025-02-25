// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/PluginAPI.h>
#include <vector>
#include <memory>

// Depend's
#include "ms_rtti.h"

// XCell
#include "XCellEvent.h"
#include "XCellSettings.h"
#include "XCellRelocator.h"

namespace XCell
{
	class Context
	{
		const F4SEInterface* _f4se_interface;
		F4SEMessagingInterface* _messages;
		F4SEPapyrusInterface* _papyrus;
		ModuleManager _modules;
		string _app_path;
		TOMLCollectionSettings _settings;
		EventInitializeDirectXLink _registered_initdx11_link;
		EventInitializePapyrusLink _registered_initvm_link;
		EventGameDataReadyLink _registered_dataready_link;
		EventGameLoadedLink _registered_loaded_link;
		EventNewGameLink _registered_newgame_link;
		EventRenderEndFrameLink _registered_endframe_link;
		EventPrepareUIDrawCuledLink _registered_uidrawculed_link;
		REL::DetourCall _graphics_listener;
		msrtti::section _section[3];
		UInt64 _base;

		HRESULT InitializeLogger() const noexcept(true);
		HRESULT InitializeCommon() noexcept(true);
		HRESULT InitializeCVar() noexcept(true);
		HRESULT InitializeRTTI() noexcept(true);
		HRESULT InitializeGraphicPending() noexcept(true);
		HRESULT InitializePapyrusPending() noexcept(true);
		HRESULT InitializeMessagePending() noexcept(true);
		HRESULT InitializeModules() noexcept(true);

		bool GetPESectionRange(const char* section, msrtti::section* data) const noexcept(true);
		
		HRESULT PerformImpl() noexcept(true);
		HRESULT ShutdownImpl() noexcept(true);
	public:
		Context(const F4SEInterface* f4se);
		virtual ~Context();

		HRESULT Initialize() noexcept(true);
		HRESULT Perform() noexcept(true);
		HRESULT Shutdown() noexcept(true);

		void Listener(Event::ListenerEventType Type, ...);
		void RegisterListeners(Module* Module, UInt32 Listeners);
		void UnregisterListeners(Module* Module, UInt32 Listeners);

		inline virtual UInt64 GetProcessBase() const noexcept(true) { return _base; }
		inline virtual msrtti::section GetPESectionText() const noexcept(true) { return _section[0]; }
		inline virtual msrtti::section GetPESectionRData() const noexcept(true) { return _section[1]; }
		inline virtual msrtti::section GetPESectionData() const noexcept(true) { return _section[2]; }

		inline virtual string GetAppPath() const noexcept(true) { return _app_path; }
		inline virtual const F4SEInterface* GetInterface() const noexcept(true) { return _f4se_interface; }

		XCPropertyReadOnly(GetProcessBase) UInt64 ProcessBase;
		XCPropertyReadOnly(GetPESectionText) msrtti::section PESectionText;
		XCPropertyReadOnly(GetPESectionRData) msrtti::section PESectionRData;
		XCPropertyReadOnly(GetPESectionData) msrtti::section PESectionData;

		XCPropertyReadOnly(GetAppPath) string AppPath;
		XCPropertyReadOnly(GetInterface) const F4SEInterface* Interface;
	};

	HRESULT __stdcall CreateContextInstance(Context** pContext, const F4SEInterface* f4se) noexcept(true);
	HRESULT __stdcall ReleaseContextInstance(const Context* pContext) noexcept(true);
	HRESULT __stdcall GetDefaultContextInstance(Context** pContext) noexcept(true);
	HRESULT __stdcall SetDefaultContextInstance(const Context* pContext) noexcept(true);
	HRESULT __stdcall RegisterModule(HMODULE hModule) noexcept(true);
	HRESULT __stdcall UnregisterModule() noexcept(true);
}



