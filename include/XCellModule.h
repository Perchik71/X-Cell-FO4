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

	class Module : public Object
	{
		bool _enabled;
	protected:
		ICriticalSection CriticalSection;
		shared_ptr<Setting> SettingMod;	
		void* Context;
	public:
		Module(void* Context, const char* Name);
		Module(void* Context, const char* Name, const shared_ptr<Setting>& SettingMod);
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

		XCPropertyReadOnly(GetEnabled) bool Enabled;
		XCPropertyReadOnly(GetSettingEnabled) bool SettingEnabled;
	protected:
		virtual HRESULT InstallImpl() = 0;
		virtual HRESULT ShutdownImpl() = 0;
	};
}