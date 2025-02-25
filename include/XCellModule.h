// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// F4SE
#include <ICriticalSection.h>
#include <f4se/PluginAPI.h>

#include <dxgi.h>
#include <d3d11.h>
#include <memory>

// XCell
#include "XCellSettings.h"

namespace XCell
{
	class Module;
	class Context;

	class EventInitializeDirectXSourceLink
	{
	public:
		typedef HRESULT(Module::*EventFunctionType)(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context, 
			IDXGISwapChain* SwapChain);

		EventInitializeDirectXSourceLink();
		EventInitializeDirectXSourceLink(const EventInitializeDirectXSourceLink&) = delete;
		EventInitializeDirectXSourceLink& operator=(const EventInitializeDirectXSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod, HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
			IDXGISwapChain* SwapChain) const noexcept(true);
	};

	class EventInitializePapyrusSourceLink
	{
	public:
		typedef HRESULT(Module::*EventFunctionType)(VirtualMachine* vm);

		EventInitializePapyrusSourceLink();
		EventInitializePapyrusSourceLink(const EventInitializePapyrusSourceLink&) = delete;
		EventInitializePapyrusSourceLink& operator=(const EventInitializePapyrusSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod, VirtualMachine* vm) const noexcept(true);
	};

	class EventGameDataReadySourceLink
	{
	public:
		typedef HRESULT(Module::*EventFunctionType)();

		EventGameDataReadySourceLink();
		EventGameDataReadySourceLink(const EventGameDataReadySourceLink&) = delete;
		EventGameDataReadySourceLink& operator=(const EventGameDataReadySourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod) const noexcept(true);
	};

	class EventGameLoadedSourceLink
	{
	public:
		typedef HRESULT(Module::*EventFunctionType)();

		EventGameLoadedSourceLink();
		EventGameLoadedSourceLink(const EventGameLoadedSourceLink&) = delete;
		EventGameLoadedSourceLink& operator=(const EventGameLoadedSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod) const noexcept(true);
	};

	class EventNewGameSourceLink
	{
	public:
		typedef HRESULT(Module::*EventFunctionType)();

		EventNewGameSourceLink();
		EventNewGameSourceLink(const EventNewGameSourceLink&) = delete;
		EventNewGameSourceLink& operator=(const EventNewGameSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod) const noexcept(true);
	};

	class EventRenderEndFrameSourceLink
	{
	public:
		typedef HRESULT(Module::* EventFunctionType)();

		EventRenderEndFrameSourceLink();
		EventRenderEndFrameSourceLink(const EventRenderEndFrameSourceLink&) = delete;
		EventRenderEndFrameSourceLink& operator=(const EventRenderEndFrameSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod) const noexcept(true);
	};

	class EventPrepareUIDrawCuledSourceLink
	{
	public:
		typedef HRESULT(Module::* EventFunctionType)();

		EventPrepareUIDrawCuledSourceLink();
		EventPrepareUIDrawCuledSourceLink(const EventPrepareUIDrawCuledSourceLink&) = delete;
		EventPrepareUIDrawCuledSourceLink& operator=(const EventPrepareUIDrawCuledSourceLink&) = delete;

		EventFunctionType OnListener;
		virtual HRESULT DoListener(const shared_ptr<Module> Mod) const noexcept(true);
	};

	enum : UInt32
	{
		XCELL_MODULE_QUERY_DIRECTX_INIT		= 1 << 0,
		XCELL_MODULE_QUERY_PAPYRUS_INIT		= 1 << 1,
		XCELL_MODULE_QUERY_DATA_READY		= 1 << 2,
		XCELL_MODULE_QUERY_GAME_LOADED		= 1 << 3,
		XCELL_MODULE_QUERY_NEW_GAME			= 1 << 4,
		XCELL_MODULE_QUERY_END_FRAME		= 1 << 5,
		XCELL_MODULE_QUERY_PREPARE_UI_DRAW	= 1 << 6,
	};

	class Module : public Object
	{
		bool _enabled;
	protected:
		UInt32 _queryEvent;
		ICriticalSection CriticalSection;
		shared_ptr<Setting> SettingMod;	
		Context* Context;
	public:
		Module(void* Context, const char* Name, UInt32 QueryEvent = 0);
		Module(void* Context, const char* Name, const shared_ptr<Setting>& SettingMod, UInt32 QueryEvent = 0);
		virtual ~Module();

		Module(const Module&) = delete;
		Module& operator=(const Module&) = delete;

		[[nodiscard]] inline virtual bool GetEnabled() const noexcept(true) { return _enabled; }
		[[nodiscard]] inline virtual bool HasSetting() const noexcept(true) { return SettingMod != nullptr; }
		[[nodiscard]] inline virtual bool GetSettingEnabled() const noexcept(true) { return SettingMod ? SettingMod->GetBool() : false; }

		virtual HRESULT Install();
		virtual HRESULT Shutdown();

		EventInitializeDirectXSourceLink InitializeDirectXLinker;
		EventInitializePapyrusSourceLink InitializePapyrusLinker;
		EventGameDataReadySourceLink GameDataReadyLinker;
		EventGameLoadedSourceLink GameLoadedLinker;
		EventNewGameSourceLink NewGameLinker;
		EventRenderEndFrameSourceLink RenderEndFrameLinker;
		EventPrepareUIDrawCuledSourceLink PrepareUIDrawCuledLinker;

		XCPropertyReadOnly(GetEnabled) bool Enabled;
		XCPropertyReadOnly(GetSettingEnabled) bool SettingEnabled;
	protected:
		virtual HRESULT InstallImpl() = 0;
		virtual HRESULT ShutdownImpl() = 0;
	};
}