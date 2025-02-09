// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/GameSettings.h>
#include <imgui.h>

// XCell
#include "XCellModule.h"

namespace XCell
{
	class XCellModuleImGUI : public Module
	{
		ID3D11DeviceContext* _DXContext;
		ID3D11RenderTargetView* _DXBackBufferView;
		::Setting* _ModMenuEffectColorR;
		::Setting* _ModMenuEffectColorG;
		::Setting* _ModMenuEffectColorB;
		ImFont* _Fonts[3];
		float _Colors[3];
		float _Sizes[2];
	public:
		static constexpr auto SourceName = "Module ImGUI";

		XCellModuleImGUI(void* Context);
		virtual ~XCellModuleImGUI() = default;

		XCellModuleImGUI(const XCellModuleImGUI&) = delete;
		XCellModuleImGUI& operator=(const XCellModuleImGUI&) = delete;

		virtual HRESULT DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context, 
			IDXGISwapChain* SwapChain);
		virtual HRESULT PrepareUIDrawCuledListener();
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
		virtual void UpdateStyles(bool Force = false);
	};
}