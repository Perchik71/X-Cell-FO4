// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModuleManager.h"

namespace XCell
{
	class Event : public Object
	{
	protected:
		ModuleManager _registered_objects;
	public:
		enum ListenerEventType
		{
			EventInitializeDirectX = 0,
			EventInitializePapyrus,
			EventGameDataReady,
			EventGameLoaded,
			EventNewGame,
		};

		Event(const char* Name);
		virtual ~Event();

		virtual HRESULT RegisterModule(const shared_ptr<Module> Mod) noexcept(true);
		virtual HRESULT UnregisterModule(const shared_ptr<Module> Mod) noexcept(true);
		virtual HRESULT UnregisterModule(const char* ModName) noexcept(true);
		virtual HRESULT Clear() noexcept(true);
		virtual UInt64 Count() const noexcept(true);
		virtual shared_ptr<Module> At(UInt64 Index) const noexcept(true);
		virtual shared_ptr<Module> FindByName(const char* ModName) const noexcept(true);

		inline shared_ptr<Module> operator[](UInt64 Index) const noexcept(true) { return At(Index); }

		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;
	};

	class EventInitializeDirectXLink : public Event
	{
	public:
		static constexpr ListenerEventType Type = EventInitializeDirectX;

		EventInitializeDirectXLink();
		EventInitializeDirectXLink(const EventInitializeDirectXLink&) = delete;
		EventInitializeDirectXLink& operator=(const EventInitializeDirectXLink&) = delete;

		virtual HRESULT Listener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context, 
			IDXGISwapChain* SwapChain) const noexcept(true);
	};

	class EventInitializePapyrusLink : public Event
	{
	public:
		static constexpr ListenerEventType Type = EventInitializePapyrus;

		EventInitializePapyrusLink();
		EventInitializePapyrusLink(const EventInitializePapyrusLink&) = delete;
		EventInitializePapyrusLink& operator=(const EventInitializePapyrusLink&) = delete;

		virtual HRESULT Listener(VirtualMachine* vm) const noexcept(true);
	};

	class EventGameDataReadyLink : public Event
	{
	public:
		static constexpr ListenerEventType Type = EventGameDataReady;

		EventGameDataReadyLink();
		EventGameDataReadyLink(const EventGameDataReadyLink&) = delete;
		EventGameDataReadyLink& operator=(const EventGameDataReadyLink&) = delete;

		virtual HRESULT Listener() const noexcept(true);
	};

	class EventGameLoadedLink : public Event
	{
	public:
		static constexpr ListenerEventType Type = EventGameLoaded;

		EventGameLoadedLink();
		EventGameLoadedLink(const EventGameLoadedLink&) = delete;
		EventGameLoadedLink& operator=(const EventGameLoadedLink&) = delete;

		virtual HRESULT Listener() const noexcept(true);
	};

	class EventNewGameLink : public Event
	{
	public:
		static constexpr ListenerEventType Type = EventNewGame;

		EventNewGameLink();
		EventNewGameLink(const EventNewGameLink&) = delete;
		EventNewGameLink& operator=(const EventNewGameLink&) = delete;

		virtual HRESULT Listener() const noexcept(true);
	};
}