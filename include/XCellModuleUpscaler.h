// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <d3d11.h>
#include <memory>

// F4SE
#include <f4se/PapyrusVM.h>

// XCell
#include "XCellState.h"
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleUpscaler : public Module
	{
		IDXGISwapChain* SwapChain;
		ID3D11DeviceContext* DeviceContext;
		unique_ptr<State> DXState;
	public:
		static constexpr auto SourceName = "Module Upscaler";

		ModuleUpscaler(void* Context);
		virtual ~ModuleUpscaler() = default;

		ModuleUpscaler(const ModuleUpscaler&) = delete;
		ModuleUpscaler& operator=(const ModuleUpscaler&) = delete;

		virtual HRESULT DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
			IDXGISwapChain* SwapChain);
		virtual HRESULT DXEndFrameListener();
		virtual HRESULT PrepareUIDrawCuledListener();
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}