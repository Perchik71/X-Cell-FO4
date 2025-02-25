// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellEvent.h"

namespace XCell
{
	Event::Event(const char* Name) :
		Object(Name)
	{}

	Event::~Event()
	{
		Clear();
	}

	HRESULT Event::RegisterModule(const shared_ptr<Module> Mod) noexcept(true)
	{
		return _registered_objects.Add(Mod);
	}

	HRESULT Event::UnregisterModule(const shared_ptr<Module> Mod) noexcept(true)
	{
		return _registered_objects.Remove(Mod);
	}

	HRESULT Event::UnregisterModule(const char* ModName) noexcept(true)
	{
		return _registered_objects.Remove(ModName);
	}

	HRESULT Event::Clear() noexcept(true)
	{
		return _registered_objects.Clear();
	}

	UInt64 Event::Count() const noexcept(true)
	{
		return _registered_objects.Size();
	}

	shared_ptr<Module> Event::At(UInt64 Index) const noexcept(true)
	{
		return _registered_objects.At(Index);
	}

	shared_ptr<Module> Event::FindByName(const char* ModName) const noexcept(true)
	{
		return _registered_objects.FindByName(ModName);
	}

	EventInitializeDirectXLink::EventInitializeDirectXLink() :
		Event("Initialize DirectX")
	{}

	HRESULT EventInitializeDirectXLink::Listener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context, 
		IDXGISwapChain* SwapChain) const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->InitializeDirectXLinker.DoListener(Module, WindowHandle, Device, Context, SwapChain)))
					_ERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventInitializePapyrusLink::EventInitializePapyrusLink() :
		Event("Initialize Papyrus")
	{}

	HRESULT EventInitializePapyrusLink::Listener(VirtualMachine* vm) const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->InitializePapyrusLinker.DoListener(Module, vm)))
					_ERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventGameDataReadyLink::EventGameDataReadyLink() :
		Event("Game Data Ready")
	{}

	HRESULT EventGameDataReadyLink::Listener() const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->GameDataReadyLinker.DoListener(Module)))
					_ERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventGameLoadedLink::EventGameLoadedLink() :
		Event("Game Loaded")
	{}

	HRESULT EventGameLoadedLink::Listener() const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->GameLoadedLinker.DoListener(Module)))
					_ERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventNewGameLink::EventNewGameLink() :
		Event("New Game")
	{}

	HRESULT EventNewGameLink::Listener() const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->NewGameLinker.DoListener(Module)))
					_ERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventRenderEndFrameLink::EventRenderEndFrameLink() :
		Event("Render End Frame")
	{}

	HRESULT EventRenderEndFrameLink::Listener() const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->RenderEndFrameLinker.DoListener(Module)))
					_FATALERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}

	EventPrepareUIDrawCuledLink::EventPrepareUIDrawCuledLink() :
		Event("Prepare UI Draw Culed")
	{}

	HRESULT EventPrepareUIDrawCuledLink::Listener() const noexcept(true)
	{
		_registered_objects.Lock();

		UInt64 Total = Count();
		for (UInt64 Index = 0; Index < Total; Index++)
		{
			auto Module = _registered_objects[Index];
			if (Module->Enabled)
			{
				if (FAILED(Module->PrepareUIDrawCuledLinker.DoListener(Module)))
					_FATALERROR("Event \"%s\" for \"%s\" processed with an error", Name.c_str(), Module->Name.c_str());
			}
		}

		_registered_objects.Unlock();
		return S_OK;
	}
}