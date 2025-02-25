// Copyright © 2024-2025 aka perchik71. All rights reserved.
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
#include "XCellCVar.h"

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

	ModuleImGUI::ModuleImGUI(void* Context) :
		Module(Context, SourceName, XCELL_MODULE_QUERY_DIRECTX_INIT | XCELL_MODULE_QUERY_PREPARE_UI_DRAW),
		_DXContext(nullptr), _DXBackBufferView(nullptr), _ModMenuEffectColorR(nullptr), _ModMenuEffectColorG(nullptr), 
		_ModMenuEffectColorB(nullptr)
	{
		ZeroMemory(_Colors, _ARRAYSIZE(_Colors));
	}

	HRESULT ModuleImGUI::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
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

	HRESULT ModuleImGUI::PrepareUIDrawCuledListener()
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
				CVarTAA->SetBool(!CVarTAA->GetBool());
			}
			ImGui::End();

			ImGui::Render();
			_DXContext->OMSetRenderTargets(1, &_DXBackBufferView, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		return S_OK;
	}

	HRESULT ModuleImGUI::InstallImpl()
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

		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&ModuleImGUI::DXListener);
		PrepareUIDrawCuledLinker.OnListener = (EventPrepareUIDrawCuledSourceLink::EventFunctionType)(&ModuleImGUI::PrepareUIDrawCuledListener);

		return S_OK;
	}

	HRESULT ModuleImGUI::ShutdownImpl()
	{
		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		return S_OK;
	}

	void ModuleImGUI::UpdateStyles(bool Force)
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

			//[ImGuiCol_Text]					= Цвет для текста, который будет использоваться для всего меню.
			//[ImGuiCol_TextDisabled]			= Цвет для "не активного/отключенного текста".
			//[ImGuiCol_WindowBg]				= Цвет заднего фона.
			//[ImGuiCol_PopupBg]				= Цвет, который используется для заднего фона в ImGui::Combo и ImGui::MenuBar.
			//[ImGuiCol_Border]					= Цвет, который используется для обводки вашего меню.
			//[ImGuiCol_BorderShadow]			= Цвет для тени обводки.
			//[ImGuiCol_FrameBg]				= Цвет для ImGui::InputText и для заднего фона ImGui::Checkbox
			//[ImGuiCol_FrameBgHovered]			= Цвет, который используется практически так же что и тот, который выше, кроме того, что он изменяет цвет при наводке на ImGui::Checkbox.
			//[ImGuiCol_FrameBgActive]			= Активный цвет.
			//[ImGuiCol_TitleBg]				= Цвет для изменения главного места в самом верху меню(там где находится название вашего "топприватногохакаинзеворлдвсес0писалсяполгода".
			//[ImGuiCol_TitleBgCollapsed]		= Свернутый цвет тайтла.
			//[ImGuiCol_TitleBgActive]			= Цвет активного окна тайтла, т.е если вы имеете меню с несколькими окнами, то этот цвет будет использоваться для окна, в котором вы будет находится на данный момент.
			//[ImGuiCol_MenuBarBg]				= Цвет для меню бара. (Не во всех сурсах видел такое, но все же)
			//[ImGuiCol_ScrollbarBg]			= Цвет для заднего фона "полоски", через которую можно "листать" функции в софте по вертикале.
			//[ImGuiCol_ScrollbarGrab]			= Цвет для сколл бара, т.е для "полоски", которая используется для передвижения меню по вертикали.
			//[ImGuiCol_ScrollbarGrabHovered]	= Цвет для "свернутого/не используемого" скролл бара.
			//[ImGuiCol_ScrollbarGrabActive]	= Цвет для "активной" деятельности в том окне, где находится скролл бар.
			//[ImGuiCol_ComboBg]				= Цвет для заднего фона для ImGui::Combo.
			//[ImGuiCol_CheckMark]				= Цвет для вашего ImGui::Checkbox.
			//[ImGuiCol_SliderGrab]				= Цвет для ползунка ImGui::SliderInt и ImGui::SliderFloat.
			//[ImGuiCol_SliderGrabActive]		= Цвет ползунка, который будет отображаться при использовании SliderFloat и SliderInt.
			//[ImGuiCol_Button]					= Цвет для кнопки.
			//[ImGuiCol_ButtonHovered]			= Цвет, при наведении на кнопку.
			//[ImGuiCol_ButtonActive]			= Используемый цвет кнопки.
			//[ImGuiCol_Header]					= Цвет для ImGui::CollapsingHeader.
			//[ImGuiCol_HeaderHovered]			= Цвет, при наведении на ImGui::CollapsingHeader.
			//[ImGuiCol_HeaderActive]			= Используемый цвет ImGui::CollapsingHeader.
			//[ImGuiCol_Column]					= Цвет для "полоски отделения" ImGui::Column и ImGui::NextColumn.
			//[ImGuiCol_ColumnHovered]			= Цвет, при наведении на "полоску отделения" ImGui::Column и ImGui::NextColumn.
			//[ImGuiCol_ColumnActive]			= Используемый цвет для "полоски отделения" ImGui::Column и ImGui::NextColumn.
			//[ImGuiCol_ResizeGrip]				= Цвет для "треугольника" в правом нижнем углу, который используется для увеличения или уменьшения размеров меню.
			//[ImGuiCol_ResizeGripHovered]		= Цвет, при наведении на "треугольника" в правом нижнем углу, который используется для увеличения или уменьшения размеров меню.
			//[ImGuiCol_ResizeGripActive]		= Используемый цвет для "треугольника" в правом нижнем углу, который используется для увеличения или уменьшения размеров меню.
			//[ImGuiCol_CloseButton]			= Цвет для кнопки - закрытия меню.
			//[ImGuiCol_CloseButtonHovered]		= Цвет, при наведении на кнопку - закрытия меню.
			//[ImGuiCol_CloseButtonActive]		= Используемый цвет для кнопки - закрытия меню.
			//<-------------------------------------------------------------------------------------------------------------->
			//Данные параметры для меня не известны, т.к не использую их на деле.
			//[ImGuiCol_PlotLines]
			//[ImGuiCol_PlotLinesHovered]
			//[ImGuiCol_PlotHistogram]
			//[ImGuiCol_PlotHistogramHovered]
			//<-------------------------------------------------------------------------------------------------------------->
			//[ImGuiCol_TextSelectedBg]			= Цвет выбранного текста, в ImGui::MenuBar.
			//[ImGuiCol_ModalWindowDarkening]	= Цвет "Затемнения окна" вашего меню.
			//Редко вижу данные обозначения, но все таки решил их сюда поместить.
			//[ImGuiCol_Tab]					= Цвет для табов в меню.
			//[ImGuiCol_TabActive]				= Активный цвет табов, т.е при нажатии на таб у вас будет этот цвет.
			//[ImGuiCol_TabHovered]				= Цвет, который будет отображаться при наведении на таб.
			//[ImGuiCol_TabSelected]			= Цвет, при котором, используется тогда, когда вы будете находится в одном из табов.
			//[ImGuiCol_TabText]				= Цвет текста, который распространяется только на табы.
			//[ImGuiCol_TabTextActive]			= Активный цвет текста для табов.

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