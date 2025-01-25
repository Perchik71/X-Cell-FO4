// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <imgui.h>

// XCell
#include "XCellModule.h"

namespace XCell
{
	class XCellModuleImGUI : public Module
	{
	public:
		static constexpr auto SourceName = "Module ImGUI";

		XCellModuleImGUI(void* Context);
		virtual ~XCellModuleImGUI() = default;

		XCellModuleImGUI(const XCellModuleImGUI&) = delete;
		XCellModuleImGUI& operator=(const XCellModuleImGUI&) = delete;

		virtual HRESULT Listener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context, 
			IDXGISwapChain* SwapChain);
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}