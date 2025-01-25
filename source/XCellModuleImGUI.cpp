// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include "XCellModuleImGUI.h"

namespace XCell
{
	XCellModuleImGUI::XCellModuleImGUI(void* Context) :
		Module(Context, SourceName)
	{}

	HRESULT XCellModuleImGUI::InstallImpl()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&XCellModuleImGUI::Listener);

		return S_OK;
	}

	HRESULT XCellModuleImGUI::ShutdownImpl()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		return S_OK;
	}

	HRESULT XCellModuleImGUI::Listener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(WindowHandle);
		ImGui_ImplDX11_Init(Device, Context);

		return S_OK;
	}
}