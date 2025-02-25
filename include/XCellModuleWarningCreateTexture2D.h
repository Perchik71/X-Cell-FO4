// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleWarningCreateTexture2D : public Module
	{
		ID3D11Device* _Device;
	public:
		static constexpr auto SourceName = "Module Warning CreateTexture2D";

		ModuleWarningCreateTexture2D(void* Context);
		virtual ~ModuleWarningCreateTexture2D() = default;

		ModuleWarningCreateTexture2D(const ModuleWarningCreateTexture2D&) = delete;
		ModuleWarningCreateTexture2D& operator=(const ModuleWarningCreateTexture2D&) = delete;

		virtual HRESULT DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
			IDXGISwapChain* SwapChain);
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}