// Copyright � 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/GameMenus.h>

#include <shlobj.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include "XCellModuleImGUI.h"
#include "XCellAssertion.h"
#include "XCellRelocator.h"
#include "XCellTableID.h"
#include "XCellVersion.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace XCell
{
	static WNDPROC _OriginalWndProc = nullptr;

	static LRESULT WINAPI HKRenderWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto Result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		if (Result) return Result;
		return _OriginalWndProc(hWnd, uMsg, wParam, lParam);
	}

	XCellModuleImGUI::XCellModuleImGUI(void* Context) :
		Module(Context, SourceName), _DXContext(nullptr), _DXBackBufferView(nullptr), 
		_ModMenuEffectColorR(nullptr), _ModMenuEffectColorG(nullptr), _ModMenuEffectColorB(nullptr)
	{
		ZeroMemory(_Colors, _ARRAYSIZE(_Colors));
	}

	HRESULT XCellModuleImGUI::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		if (!IsWindow(WindowHandle) || !Device || !Context || !SwapChain)
			return E_FAIL;

		auto Prefs = *g_iniPrefSettings.GetPtr();
		if (!Prefs)
			return E_FAIL;

		_ModMenuEffectColorR = Prefs->Get("fModMenuEffectColorR:VATS");
		_ModMenuEffectColorG = Prefs->Get("fModMenuEffectColorG:VATS");
		_ModMenuEffectColorB = Prefs->Get("fModMenuEffectColorB:VATS");

		if (!_ModMenuEffectColorR || !_ModMenuEffectColorG || !_ModMenuEffectColorB)
			return E_FAIL;

		UpdateStyles(true);

		_DXContext = Context;
		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(WindowHandle);
		ImGui_ImplDX11_Init(Device, Context);

		_OriginalWndProc = (WNDPROC)GetClassLongPtrA(WindowHandle, GCLP_WNDPROC);
		XCAssert(_OriginalWndProc);
		_OriginalWndProc = (WNDPROC)REL::Impl::DetourJump((UInt64)_OriginalWndProc,
			(UInt64)HKRenderWndProc);

		RECT rc;
		GetWindowRect(WindowHandle, &rc);
		_Sizes[0] = rc.right;
		_Sizes[1] = rc.bottom;

		ID3D11Texture2D* pBackBuffer;
		SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		if (pBackBuffer)
		{
			if (FAILED(Device->CreateRenderTargetView(pBackBuffer, nullptr, &_DXBackBufferView)))
			{
				pBackBuffer->Release();
				return E_FAIL;
			}
			pBackBuffer->Release();
		}
		else
			return E_FAIL;

		return S_OK;
	}

	HRESULT XCellModuleImGUI::PrepareUIDrawCuledListener()
	{
		// Show only menu save/load/quit etc
		if ((*g_ui.GetPtr())->IsMenuOpen("PauseMenu"))
		{
			UpdateStyles();

			// IMGUI DRAWING
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// TODO
			ImGui::SetNextWindowSize({ _Sizes[0], 120 });
			ImGui::SetNextWindowPos({ 0.0f, 0.0f });
			ImGui::Begin("X-Cell", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration);
			
			ImGui::PushFont(_Fonts[2]);
			ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "X-Cell v%u.%u (build %u)", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(_Fonts[1]);
			ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "X-Cell v%u.%u (build %u)", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushFont(_Fonts[0]);
			ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "X-Cell v%u.%u (build %u)", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
			ImGui::PopFont();

			
			ImGui::Text("Hello World");
			ImGui::TextDisabled("Hello World");
			if (ImGui::Button("Press"))
			{

			}
			ImGui::End();

			ImGui::Render();
			_DXContext->OMSetRenderTargets(1, &_DXBackBufferView, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		return S_OK;
	}

	HRESULT XCellModuleImGUI::InstallImpl()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;		// Hide cursor

		io.Fonts->AddFontDefault();

		char path[MAX_PATH];
		if (FAILED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, path)))
			return E_FAIL;
		string ps(path);

		_Fonts[0] = io.Fonts->AddFontFromFileTTF((ps + "\\tahoma.ttf").c_str(), 14.0f);
		_Fonts[1] = io.Fonts->AddFontFromFileTTF((ps + "\\tahoma.ttf").c_str(), 16.0f);
		_Fonts[2] = io.Fonts->AddFontFromFileTTF((ps + "\\tahomabd.ttf").c_str(), 16.0f);
		if (!_Fonts[0] || !_Fonts[1] || !_Fonts[2])
			return E_FAIL;

		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&XCellModuleImGUI::DXListener);
		PrepareUIDrawCuledLinker.OnListener = (EventPrepareUIDrawCuledSourceLink::EventFunctionType)(&XCellModuleImGUI::PrepareUIDrawCuledListener);

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

	void XCellModuleImGUI::UpdateStyles(bool Force)
	{
		if (Force || 
			((abs(_Colors[0] - _ModMenuEffectColorR->data.f32) > 0.001) || 
			 (abs(_Colors[1] - _ModMenuEffectColorG->data.f32) > 0.001) ||
			 (abs(_Colors[2] - _ModMenuEffectColorB->data.f32) > 0.001)))
		{
			_Colors[0] = _ModMenuEffectColorR->data.f32;
			_Colors[1] = _ModMenuEffectColorG->data.f32;
			_Colors[2] = _ModMenuEffectColorB->data.f32;
			
			auto& _Style = ImGui::GetStyle();

			_Style.WindowBorderSize = 1.8f;
			_Style.WindowRounding = 0.0f;
			_Style.AntiAliasedLines = true;

			//[ImGuiCol_Text]					= ���� ��� ������, ������� ����� �������������� ��� ����� ����.
			//[ImGuiCol_TextDisabled]			= ���� ��� "�� ���������/������������ ������".
			//[ImGuiCol_WindowBg]				= ���� ������� ����.
			//[ImGuiCol_PopupBg]				= ����, ������� ������������ ��� ������� ���� � ImGui::Combo � ImGui::MenuBar.
			//[ImGuiCol_Border]					= ����, ������� ������������ ��� ������� ������ ����.
			//[ImGuiCol_BorderShadow]			= ���� ��� ���� �������.
			//[ImGuiCol_FrameBg]				= ���� ��� ImGui::InputText � ��� ������� ���� ImGui::Checkbox
			//[ImGuiCol_FrameBgHovered]			= ����, ������� ������������ ����������� ��� �� ��� � ���, ������� ����, ����� ����, ��� �� �������� ���� ��� ������� �� ImGui::Checkbox.
			//[ImGuiCol_FrameBgActive]			= �������� ����.
			//[ImGuiCol_TitleBg]				= ���� ��� ��������� �������� ����� � ����� ����� ����(��� ��� ��������� �������� ������ "������������������������������0��������������".
			//[ImGuiCol_TitleBgCollapsed]		= ��������� ���� ������.
			//[ImGuiCol_TitleBgActive]			= ���� ��������� ���� ������, �.� ���� �� ������ ���� � ����������� ������, �� ���� ���� ����� �������������� ��� ����, � ������� �� ����� ��������� �� ������ ������.
			//[ImGuiCol_MenuBarBg]				= ���� ��� ���� ����. (�� �� ���� ������ ����� �����, �� ��� ��)
			//[ImGuiCol_ScrollbarBg]			= ���� ��� ������� ���� "�������", ����� ������� ����� "�������" ������� � ����� �� ���������.
			//[ImGuiCol_ScrollbarGrab]			= ���� ��� ����� ����, �.� ��� "�������", ������� ������������ ��� ������������ ���� �� ���������.
			//[ImGuiCol_ScrollbarGrabHovered]	= ���� ��� "����������/�� �������������" ������ ����.
			//[ImGuiCol_ScrollbarGrabActive]	= ���� ��� "��������" ������������ � ��� ����, ��� ��������� ������ ���.
			//[ImGuiCol_ComboBg]				= ���� ��� ������� ���� ��� ImGui::Combo.
			//[ImGuiCol_CheckMark]				= ���� ��� ������ ImGui::Checkbox.
			//[ImGuiCol_SliderGrab]				= ���� ��� �������� ImGui::SliderInt � ImGui::SliderFloat.
			//[ImGuiCol_SliderGrabActive]		= ���� ��������, ������� ����� ������������ ��� ������������� SliderFloat � SliderInt.
			//[ImGuiCol_Button]					= ���� ��� ������.
			//[ImGuiCol_ButtonHovered]			= ����, ��� ��������� �� ������.
			//[ImGuiCol_ButtonActive]			= ������������ ���� ������.
			//[ImGuiCol_Header]					= ���� ��� ImGui::CollapsingHeader.
			//[ImGuiCol_HeaderHovered]			= ����, ��� ��������� �� ImGui::CollapsingHeader.
			//[ImGuiCol_HeaderActive]			= ������������ ���� ImGui::CollapsingHeader.
			//[ImGuiCol_Column]					= ���� ��� "������� ���������" ImGui::Column � ImGui::NextColumn.
			//[ImGuiCol_ColumnHovered]			= ����, ��� ��������� �� "������� ���������" ImGui::Column � ImGui::NextColumn.
			//[ImGuiCol_ColumnActive]			= ������������ ���� ��� "������� ���������" ImGui::Column � ImGui::NextColumn.
			//[ImGuiCol_ResizeGrip]				= ���� ��� "������������" � ������ ������ ����, ������� ������������ ��� ���������� ��� ���������� �������� ����.
			//[ImGuiCol_ResizeGripHovered]		= ����, ��� ��������� �� "������������" � ������ ������ ����, ������� ������������ ��� ���������� ��� ���������� �������� ����.
			//[ImGuiCol_ResizeGripActive]		= ������������ ���� ��� "������������" � ������ ������ ����, ������� ������������ ��� ���������� ��� ���������� �������� ����.
			//[ImGuiCol_CloseButton]			= ���� ��� ������ - �������� ����.
			//[ImGuiCol_CloseButtonHovered]		= ����, ��� ��������� �� ������ - �������� ����.
			//[ImGuiCol_CloseButtonActive]		= ������������ ���� ��� ������ - �������� ����.
			//<-------------------------------------------------------------------------------------------------------------->
			//������ ��������� ��� ���� �� ��������, �.� �� ��������� �� �� ����.
			//[ImGuiCol_PlotLines]
			//[ImGuiCol_PlotLinesHovered]
			//[ImGuiCol_PlotHistogram]
			//[ImGuiCol_PlotHistogramHovered]
			//<-------------------------------------------------------------------------------------------------------------->
			//[ImGuiCol_TextSelectedBg]			= ���� ���������� ������, � ImGui::MenuBar.
			//[ImGuiCol_ModalWindowDarkening]	= ���� "���������� ����" ������ ����.
			//����� ���� ������ �����������, �� ��� ���� ����� �� ���� ���������.
			//[ImGuiCol_Tab]					= ���� ��� ����� � ����.
			//[ImGuiCol_TabActive]				= �������� ���� �����, �.� ��� ������� �� ��� � ��� ����� ���� ����.
			//[ImGuiCol_TabHovered]				= ����, ������� ����� ������������ ��� ��������� �� ���.
			//[ImGuiCol_TabSelected]			= ����, ��� �������, ������������ �����, ����� �� ������ ��������� � ����� �� �����.
			//[ImGuiCol_TabText]				= ���� ������, ������� ���������������� ������ �� ����.
			//[ImGuiCol_TabTextActive]			= �������� ���� ������ ��� �����.

			_Style.Colors[ImGuiCol_Text]				= { _Colors[0], _Colors[1], _Colors[2], 1.0f };
			_Style.Colors[ImGuiCol_TextDisabled]		= { _Colors[0] * 0.5f, _Colors[1] * 0.5f, _Colors[2] * 0.5f, 1.0f };
			_Style.Colors[ImGuiCol_WindowBg]			= { _Colors[0] * 0.15f, _Colors[1] * 0.15f, _Colors[2] * 0.15f, 0.6f };
			_Style.Colors[ImGuiCol_PopupBg]				= { _Colors[0] * 0.15f, _Colors[1] * 0.15f, _Colors[2] * 0.15f, 1.0f };
			_Style.Colors[ImGuiCol_Border]				= _Style.Colors[ImGuiCol_Text];
			_Style.Colors[ImGuiCol_BorderShadow]		= { 0.0f, 0.0f, 0.0f, 0.7f };
			_Style.Colors[ImGuiCol_TitleBg]				= _Style.Colors[ImGuiCol_WindowBg];
			_Style.Colors[ImGuiCol_TitleBgCollapsed]	= { _Colors[0] * 0.5f, _Colors[1] * 0.5f, _Colors[2] * 0.5f, 1.0f };
			_Style.Colors[ImGuiCol_TitleBgActive]		= { _Colors[0] * 0.6f, _Colors[1] * 0.6f, _Colors[2] * 0.6f, 1.0f };
			_Style.Colors[ImGuiCol_Button]				= { _Colors[0] * 0.4f, _Colors[1] * 0.4f, _Colors[2] * 0.4f, 1.0f };
			_Style.Colors[ImGuiCol_ButtonHovered]		= { _Colors[0] * 0.5f, _Colors[1] * 0.5f, _Colors[2] * 0.5f, 1.0f };
			_Style.Colors[ImGuiCol_ButtonActive]		= { _Colors[0] * 0.6f, _Colors[1] * 0.6f, _Colors[2] * 0.6f, 1.0f };
		}
	}
}