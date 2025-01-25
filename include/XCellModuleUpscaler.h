// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// F4SE
#include <f4se/PapyrusVM.h>

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleUpscaler : public Module
	{
	public:
		static constexpr auto SourceName = "Module Upscaler";

		XCellModuleUpscaler(void* Context);
		virtual ~XCellModuleUpscaler() = default;

		XCellModuleUpscaler(const XCellModuleUpscaler&) = delete;
		XCellModuleUpscaler& operator=(const XCellModuleUpscaler&) = delete;

		virtual HRESULT DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
			IDXGISwapChain* SwapChain);
		virtual HRESULT VMListener(VirtualMachine* VM);
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}