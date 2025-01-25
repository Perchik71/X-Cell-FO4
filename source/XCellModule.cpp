// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModule.h"

namespace XCell
{
	EventInitializeDirectXSourceLink::EventInitializeDirectXSourceLink() :
		OnListener(nullptr)
	{}

	HRESULT EventInitializeDirectXSourceLink::DoListener(const shared_ptr<Module> Mod, HWND WindowHandle, ID3D11Device* Device, 
		ID3D11DeviceContext* Context, IDXGISwapChain* SwapChain) const noexcept(true)
	{
		if (OnListener)
		{
			auto m = Mod.get();
			if (m->HasSetting() && !m->SettingEnabled)
				// Skip
				return S_FALSE;
			// Run
			return ((*m).*OnListener)(WindowHandle, Device, Context, SwapChain);
		}
		return E_FAIL;
	}

	EventInitializePapyrusSourceLink::EventInitializePapyrusSourceLink() :
		OnListener(nullptr)
	{}

	HRESULT EventInitializePapyrusSourceLink::DoListener(const shared_ptr<Module> Mod, VirtualMachine* vm) const noexcept(true)
	{
		if (OnListener)
		{
			auto m = Mod.get();
			if (m->HasSetting() && !m->SettingEnabled)
				// Skip
				return S_FALSE;
			// Run
			return ((*m).*OnListener)(vm);
		}
		return E_FAIL;
	}

	EventGameDataReadySourceLink::EventGameDataReadySourceLink() :
		OnListener(nullptr)
	{}

	HRESULT EventGameDataReadySourceLink::DoListener(const shared_ptr<Module> Mod) const noexcept(true)
	{
		if (OnListener)
		{
			auto m = Mod.get();
			if (m->HasSetting() && !m->SettingEnabled)
				// Skip
				return S_FALSE;
			// Run
			return ((*m).*OnListener)();
		}
		return E_FAIL;
	}

	EventGameLoadedSourceLink::EventGameLoadedSourceLink() :
		OnListener(nullptr)
	{}

	HRESULT EventGameLoadedSourceLink::DoListener(const shared_ptr<Module> Mod) const noexcept(true)
	{
		if (OnListener)
		{
			auto m = Mod.get();
			if (m->HasSetting() && !m->SettingEnabled)
				// Skip
				return S_FALSE;
			// Run
			return ((*m).*OnListener)();
		}
		return E_FAIL;
	}

	EventNewGameSourceLink::EventNewGameSourceLink() :
		OnListener(nullptr)
	{}

	HRESULT EventNewGameSourceLink::DoListener(const shared_ptr<Module> Mod) const noexcept(true)
	{
		if (OnListener)
		{
			auto m = Mod.get();
			if (m->HasSetting() && !m->SettingEnabled)
				// Skip
				return S_FALSE;
			// Run
			return ((*m).*OnListener)();
		}
		return E_FAIL;
	}

	Module::Module(void* Context, const char* Name) :
		Object(Name), Context(Context), SettingMod(nullptr), _enabled(false)
	{}

	Module::Module(void* Context, const char* Name, const shared_ptr<Setting>& SettingMod) :
		Object(Name), Context(Context), SettingMod(SettingMod), _enabled(false)
	{
		if (SettingMod && !SettingMod->CheckValidValueType(Setting::stBool))
		{
			_ERROR("The \"%s\" setting type should be boolean.", SettingMod->Name.c_str());
			this->SettingMod = nullptr;
		}
	}

	Module::~Module()
	{
		Shutdown();
	}

	HRESULT Module::Install()
	{
		if (_enabled || (SettingMod && !SettingMod->GetBool()))
			return S_FALSE;

		auto Result = InstallImpl();
		_enabled = SUCCEEDED(Result);
		return Result;
	}

	HRESULT Module::Shutdown()
	{
		if (!_enabled || (SettingMod && !SettingMod->GetBool()))
			return S_FALSE;

		auto Result = ShutdownImpl();
		_enabled = FAILED(Result);
		return Result;
	}
}