// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <comdef.h>
#include <ShlObj_core.h>
#include <vector>

#define NOUSE_DX12_2 1
#define NOUSE_HOOK_CLEAR_DEPTH 1

// F4SE
#include <f4se/PapyrusNativeFunctions.h>
#include <f4se/GameSettings.h>

// XCell
#include "XCellCommonType.h"
#include "XCellAssertion.h"
#include "XCellModuleUpscaler.h"
#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellPostProcessor.h"

namespace XCell
{
	using namespace Microsoft::WRL;

	static char sAntiAliasingDef[] = /*""*/"FXAA";
	static char sAntiAliasingOrig[] = "TAA";
	static const char DXErrorMessage[] = "Graphics card does not meet the minimum requirements. See log.";

	decltype(&CreateWindowExA) gOldCreateWindow = 0;
	decltype(&GetClientRect) gOldGetClientRect = 0;
	decltype(&GetWindowRect) gOldGetWindowRect = 0;
	decltype(&SetWindowPos) gOldSetWindowPos = 0;

	UInt64 gOldFileFinderConstructor = 0;
	UInt64 gOldCreateDepthStencilView = 0;
	UInt64 gOldClearDepthStencilView = 0;
	UInt32 gRndSourceWidth = 0, gRndSourceHeight = 0;
	vector<ID3D11Resource*> gDepthStencilResources;
	ID3D11DepthStencilView* gDepthStencilView;

	static HRESULT WINAPI HKD3DCreateDepthStencilView(ID3D11Device* This, ID3D11Resource* pResource,
		const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView)
	{
		if (find(gDepthStencilResources.begin(), gDepthStencilResources.end(), pResource) == gDepthStencilResources.end())
		{
			D3D11_TEXTURE2D_DESC dsc;
			((ID3D11Texture2D*)pResource)->GetDesc(&dsc);

			// _MESSAGE("[DBG] Create depth and stencil: %ux%u 0x%X", dsc.Width, dsc.Height, dsc.Format);

			// In FO4 depth frame - half native frame sizes
			if ((dsc.Width == (gRndSourceWidth >> 1)) && (dsc.Height == (gRndSourceHeight >> 1)) && !gDepthStencilResources.size())
			{
				_MESSAGE("New depth and stencil texture: 0x%X 0x%X 0x%X", pDesc->Format, pDesc->Flags, pDesc->ViewDimension);

				auto hr = XCFastCall<HRESULT>(gOldCreateDepthStencilView, This, pResource, pDesc, ppDepthStencilView);
				if (SUCCEEDED(hr))
				{
					gDepthStencilView = *ppDepthStencilView;
					gDepthStencilResources.push_back(pResource);
				}
				return hr;
			}
		}

		return XCFastCall<HRESULT>(gOldCreateDepthStencilView, This, pResource, pDesc, ppDepthStencilView);
	}

	static void WINAPI HKD3DClearDepthStencilView(ID3D11DeviceContext* This, ID3D11DepthStencilView* pDepthStencilView,
		UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
	{
		if (gDepthStencilView != pDepthStencilView)
			return XCFastCall<void>(gOldClearDepthStencilView, This, pDepthStencilView, ClearFlags, Depth, Stencil);
	}  

	static BOOL WINAPI HKSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
	{
		// nope

		return TRUE;
	}

	static BOOL WINAPI HKGetClientRect(HWND hWnd, LPRECT pRect)
	{
		auto bRet = gOldGetClientRect(hWnd, pRect);

		if (bRet && (gDirectXData->window == hWnd))
		{
			pRect->right = gRndSourceWidth;
			pRect->bottom = gRndSourceHeight;
		}

		return bRet;
	}

	static BOOL WINAPI HKGetWindowRect(HWND hWnd, LPRECT pRect)
	{
		auto bRet = gOldGetWindowRect(hWnd, pRect);

		if (bRet && (gDirectXData->window == hWnd))
		{
			pRect->right = gRndSourceWidth + pRect->left;
			pRect->bottom = gRndSourceHeight + pRect->top;
		}

		return bRet;
	}

	static HWND WINAPI HKCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
		DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
		HINSTANCE hInstance, LPVOID lpParam)
	{
		if (!strcmp("Fallout4", lpClassName))
		{
			auto SettingScale = CVarDisplayScale->GetFloat();

			long bTopMostWindow = 1;
			Game::XCReadINISettingGameInt(Game::gINIGameSettingPref, "bTopMostWindow:Display", 1, &bTopMostWindow);

			dwExStyle &= ~WS_EX_TOPMOST;
			if (bTopMostWindow) dwExStyle |= WS_EX_TOPMOST;

			gRndSourceWidth = nWidth;
			gRndSourceHeight = nHeight;
			UINT uWidth = nWidth / SettingScale;
			UINT uHeight = nHeight / SettingScale;
			gDirectXData->window = gOldCreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle,
				X, Y, uWidth, uHeight, hWndParent, hMenu, hInstance, lpParam);
			return gDirectXData->window;
		}

		return gOldCreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle,
			X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	static UInt64 WINAPI HKFileFinderConstructor(UInt64 Unk0, UInt64 Unk1, UInt64 Unk2, UInt64 Unk3)
	{
		auto SettingSizeW = (::Setting*)REL::ID(230);
		auto SettingSizeH = (::Setting*)REL::ID(231);
		auto SettingScale = CVarDisplayScale->GetFloat();

		// Insurance in case of a repeat call
		if (!gDirectXData->window)
		{
			_MESSAGE("Upscale: Settings loaded %ux%u scale %.3f", 
				SettingSizeW->data.u32, SettingSizeH->data.u32, SettingScale);

			SettingSizeW->data.u32 = (UInt32)(SettingSizeW->data.u32 * SettingScale);
			SettingSizeH->data.u32 = (UInt32)(SettingSizeH->data.u32 * SettingScale);

			_MESSAGE("Upscale: Changing the display settings %ux%u according to scale %.3f", 
				SettingSizeW->data.u32, SettingSizeH->data.u32, SettingScale);
		}

		return XCFastCall<UInt64>(gOldFileFinderConstructor, Unk0, Unk1, Unk2, Unk3);
	}

	static HRESULT WINAPI HKD3D11CreateDeviceAndSwapChain(
		IDXGIAdapter* pAdapter,
		D3D_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		const D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT FeatureLevels,
		UINT SDKVersion,
		const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
		IDXGISwapChain** ppSwapChain,
		ID3D11Device** ppDevice,
		D3D_FEATURE_LEVEL* pFeatureLevel,
		ID3D11DeviceContext** ppImmediateContext)
	{
		if (!pSwapChainDesc || !ppSwapChain || !ppImmediateContext || !ppDevice ||
			!IsWindow(pSwapChainDesc->OutputWindow))
			return E_INVALIDARG;

		D3D_FEATURE_LEVEL FeatureLevel;
		const D3D_FEATURE_LEVEL TestFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		
#if NOUSE_DX12_2
		auto hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, TestFeatureLevels,
			_ARRAYSIZE(TestFeatureLevels), D3D11_SDK_VERSION, ppDevice, &FeatureLevel,
			ppImmediateContext);
		if (FAILED(hr))
		{
			_ERROR("D3D11CreateDevice return failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		_MESSAGE("Created D3D11 device with feature level %X...", FeatureLevel);

		ComPtr<IDXGIFactory> Factory;
		hr = CreateDXGIFactory(IID_IDXGIFactory, (void**)Factory.GetAddressOf());
		if (FAILED(hr))
		{
			_MESSAGE("Failed to create IDXGIFactory 0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		DXGI_SWAP_CHAIN_DESC scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		scDesc.BufferDesc.Width = static_cast<UINT>(gRndSourceWidth);
		scDesc.BufferDesc.Height = static_cast<UINT>(gRndSourceHeight);
		scDesc.BufferDesc.Format = pSwapChainDesc->BufferDesc.Format;
		scDesc.BufferUsage = pSwapChainDesc->BufferUsage | DXGI_USAGE_UNORDERED_ACCESS;
		scDesc.BufferCount = pSwapChainDesc->BufferCount;
		scDesc.OutputWindow = pSwapChainDesc->OutputWindow;
		scDesc.SampleDesc.Count = 1;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// FLIP better upscaling mode (maybe need Win8.1 or newer)
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scDesc.Windowed = pSwapChainDesc->Windowed;
		scDesc.BufferDesc.RefreshRate.Denominator = pSwapChainDesc->BufferDesc.RefreshRate.Denominator;
		scDesc.BufferDesc.RefreshRate.Numerator = pSwapChainDesc->BufferDesc.RefreshRate.Numerator;

		auto SettingScale = CVarDisplayScale->GetFloat();
		_MESSAGE("Render Window: (0x%016llX) %ux%u", (UInt64)pSwapChainDesc->OutputWindow, 
			(UINT)(gRndSourceWidth / SettingScale), (UINT)(gRndSourceHeight / SettingScale));

		hr = Factory->CreateSwapChain(gDirectXData->device, &scDesc, ppSwapChain);
		if (FAILED(hr))
		{
			_MESSAGE("CreateSwapChain returned failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}
#else
		auto hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, TestFeatureLevels,
			_ARRAYSIZE(TestFeatureLevels), D3D11_SDK_VERSION, ppDevice, &FeatureLevel, ppImmediateContext);
		if (FAILED(hr))
		{
			_ERROR("D3D11CreateDevice return failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		_MESSAGE("Created D3D11 device with feature level %X...", FeatureLevel);

		hr = (*ppDevice)->QueryInterface<ID3D11Device2>(&gDirectXData->device);
		if (FAILED(hr))
		{
			_ERROR("Initialization error DirectX 11.2 (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		gRenderWindow.Device->GetImmediateContext2(&gDirectXData->context);

		ComPtr<IDXGIFactory2> Factory;
		hr = CreateDXGIFactory2(0, IID_IDXGIFactory2, (void**)Factory.GetAddressOf());
		if (FAILED(hr))
		{
			_MESSAGE("Failed to create IDXGIFactory2 0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		DXGI_SWAP_CHAIN_DESC1 scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

		scDesc.Width = static_cast<UINT>(gRndSourceWidth);
		scDesc.Height = static_cast<UINT>(gRndSourceHeight);
		scDesc.Format = pSwapChainDesc->BufferDesc.Format;
		scDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		scDesc.BufferUsage = pSwapChainDesc->BufferUsage | DXGI_USAGE_UNORDERED_ACCESS;
		scDesc.BufferCount = pSwapChainDesc->BufferCount;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scDesc.Scaling = DXGI_SCALING_STRETCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC scFullScreenDesc;
		ZeroMemory(&scFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

		scFullScreenDesc.RefreshRate.Numerator = (UINT)CVarMaxFrameRateVSync->GetUnsignedInt();
		scFullScreenDesc.RefreshRate.Denominator = (UINT)CVarVSync->GetBool();
		scFullScreenDesc.Windowed = pSwapChainDesc->Windowed;
		scFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		auto SettingScale = CVarDisplayScale->GetFloat();
		_MESSAGE("Render Window: (0x%016llX) %ux%u", (UInt64)pSwapChainDesc->OutputWindow,
			(UINT)(gRndSourceWidth / SettingScale), (UINT)(gRndSourceHeight / SettingScale));
		
		hr = Factory->CreateSwapChainForHwnd(gRenderWindow.Device, pSwapChainDesc->OutputWindow, &scDesc,
			&scFullScreenDesc, nullptr, (IDXGISwapChain1**)&gDirectXData->swap_chain);
		if (FAILED(hr))
		{
			_MESSAGE("CreateSwapChainForHwnd returned failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			XCAssertWithFormattedMessage(false, "%s", DXErrorMessage);

			return E_FAIL;
		}

		// Change the region of the swap chain that will be presented to the screen
		gRenderWindow.SwapChain->SetSourceSize(gDirectXData->width, gDirectXData->height);

		*ppSwapChain = gRenderWindow.SwapChain;
		*ppDevice = gRenderWindow.Device;
		*ppImmediateContext = gRenderWindow.DeviceContext;
#endif
		auto vtableCreateDepthStencilView = reinterpret_cast<UInt64*>((uintptr_t)(*((UInt64*)(*ppDevice))) + (10 * sizeof(void*)));
		gOldCreateDepthStencilView = REL::Impl::DetourJump(*vtableCreateDepthStencilView, (UInt64)&HKD3DCreateDepthStencilView);
#if NOUSE_HOOK_CLEAR_DEPTH == 0
		auto vtableClearDepthStencilView = reinterpret_cast<UInt64*>((uintptr_t)(*((UInt64*)(*ppImmediateContext))) + (53 * sizeof(void*)));
		gOldClearDepthStencilView = REL::Impl::DetourJump(*vtableClearDepthStencilView, (UInt64)&HKD3DClearDepthStencilView);
#endif

		_MESSAGE("HKD3D11CreateDeviceAndSwapChain: Successed");

		if (pFeatureLevel)
			*pFeatureLevel = FeatureLevel;

		return S_OK;
	}

	static FARPROC WINAPI HKGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{
		if (!strcmp(lpProcName, "D3D11CreateDeviceAndSwapChain"))
			// This is our client, it's time to deceive him
			return reinterpret_cast<FARPROC>(&HKD3D11CreateDeviceAndSwapChain);
		return GetProcAddress(hModule, lpProcName);
	}

	ModuleUpscaler::ModuleUpscaler(void* Context) :
		Module(Context, SourceName, CVarUpscaler, XCELL_MODULE_QUERY_DIRECTX_INIT
#if NOUSE_HOOK_CLEAR_DEPTH == 0
			| XCELL_MODULE_QUERY_END_FRAME
#endif
			| XCELL_MODULE_QUERY_PREPARE_UI_DRAW),
		SwapChain(nullptr)
	{}

	HRESULT ModuleUpscaler::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		this->SwapChain = SwapChain;
		this->DeviceContext = Context;
		this->DXState = make_unique<State>(DeviceContext);

		if (GlobalPostProcessor)
		{
			delete GlobalPostProcessor;
			GlobalPostProcessor = nullptr;
		}

		auto SettingTAA = (::Setting*)REL::ID(245);
		if (CVarNoUseTAA->GetBool())
			SettingTAA->data.s = sAntiAliasingDef;
		else
			SettingTAA->data.s = sAntiAliasingOrig;

		if (CVarNoUseTAA->GetBool())
		{
			GlobalPostProcessor = new PostProcessor(Device, Context, SwapChain, WindowHandle);
			GlobalPostProcessor->Install();
		}
		else
			_queryEvent &= ~(XCELL_MODULE_QUERY_PREPARE_UI_DRAW | XCELL_MODULE_QUERY_END_FRAME);

		return S_OK;
	}

	HRESULT ModuleUpscaler::DXEndFrameListener()
	{
		//XCFastCall<void>(gOldClearDepthStencilView, DeviceContext, pDepthStencilView, ClearFlags, Depth, Stencil);

		return S_OK;
	}

	HRESULT ModuleUpscaler::PrepareUIDrawCuledListener()
	{
		if (CVarNoUseTAA->GetBool())
		{
			DXState->PushState();

			ID3D11RenderTargetView* OldRenderTarget;
			ID3D11DepthStencilView* OlgDepthStencil;
			DeviceContext->OMGetRenderTargets(1, &OldRenderTarget, &OlgDepthStencil);
			// disable any RTs in case our input texture is still bound; otherwise using it as a view will fail
			DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

			ComPtr<ID3D11Texture2D> _texture;
			if (FAILED(SwapChain->GetBuffer(0, IID_ID3D11Texture2D, reinterpret_cast<void**>(_texture.GetAddressOf()))))
			{
				DXState->PopState();
				return E_FAIL;
			}

			DeviceContext->OMSetRenderTargets(1, &OldRenderTarget, nullptr);
			GlobalPostProcessor->Processing(OldRenderTarget, _texture.Get(),
				!gDepthStencilResources.size() ? nullptr : gDepthStencilResources[0]);
			DXState->PopState();
		}

		return S_OK;
	}

	HRESULT ModuleUpscaler::InstallImpl()
	{
		CVarLodMipBias->SetFloat(0);
		CVarMaxAnisotropy->SetUnsignedInt(0);

		bool D3D11HookDetected = false;
		auto hD3D11Module = GetModuleHandleA("d3d11.dll");
		if (!hD3D11Module)
		{
			// Wow, imposible
			return E_FAIL;
		}

		string sD3D11ModulePath;
		{
			char szD3D11ModulePath[MAX_PATH];
			GetModuleFileNameA(hD3D11Module, szD3D11ModulePath, MAX_PATH);
			sD3D11ModulePath = szD3D11ModulePath;
		}

		char szSystemPath[MAX_PATH];
		D3D11HookDetected = _strnicmp(sD3D11ModulePath.c_str(), szSystemPath, GetSystemDirectoryA(szSystemPath, MAX_PATH)) != 0;
		auto pvContext = (XCell::Context*)Context;

		if (D3D11HookDetected)
		{
			// The victim uses ENB, Reshade or DLLLoader, let's show off

			_MESSAGE("Library \"d3d11.dll\" injection detected");

			// At this stage, the original d3d11.dll has not been loaded yet.

			// ENB uses LoadLibrary/GetProcAddress, let's intercept it
			if (!REL::Impl::DetourIAT(
				(UInt64)hD3D11Module,
				"kernel32.dll",
				"GetProcAddress",
				(UInt64)&HKGetProcAddress))
				// If we fail, then it's not ENB or Boris has become arrogant
				return E_FAIL;
		}
		else
			// A person does not use ENB, other nonsense
			REL::Impl::DetourIAT(
				pvContext->ProcessBase,
				"d3d11.dll", 
				"D3D11CreateDeviceAndSwapChain", 
				(UInt64)&HKD3D11CreateDeviceAndSwapChain);

		// I'll intercept this function, at this stage, the settings have already been loaded, but they have not yet been accepted.
		gOldFileFinderConstructor = REL::Impl::DetourJump(REL::ID(240), (UInt64)&HKFileFinderConstructor);
		// I'll deceive him and send him the dimensions that the window "seems" to match.
		auto hUser32Module = GetModuleHandleA("user32.dll");
		if (hUser32Module)
		{
			*(FARPROC*)&gOldCreateWindow = GetProcAddress(hUser32Module, "CreateWindowExA");
			*(FARPROC*)&gOldGetClientRect = GetProcAddress(hUser32Module, "GetClientRect");
			*(FARPROC*)&gOldGetWindowRect = GetProcAddress(hUser32Module, "GetWindowRect");
			*(FARPROC*)&gOldSetWindowPos = GetProcAddress(hUser32Module, "SetWindowPos");

			*(UInt64*)&gOldGetClientRect = REL::Impl::DetourJump((UInt64)gOldGetClientRect, (UInt64)&HKGetClientRect);
			*(UInt64*)&gOldGetWindowRect = REL::Impl::DetourJump((UInt64)gOldGetWindowRect, (UInt64)&HKGetWindowRect);
			*(UInt64*)&gOldCreateWindow = REL::Impl::DetourJump((UInt64)gOldCreateWindow, (UInt64)&HKCreateWindowExA);
			*(UInt64*)&gOldSetWindowPos = REL::Impl::DetourJump((UInt64)gOldSetWindowPos, (UInt64)&HKSetWindowPos);
		}

		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&ModuleUpscaler::DXListener);
#if NOUSE_HOOK_CLEAR_DEPTH == 0
		RenderEndFrameLinker.OnListener = (EventRenderEndFrameSourceLink::EventFunctionType)(&ModuleUpscaler::DXEndFrameListener);
#endif
		PrepareUIDrawCuledLinker.OnListener = (EventPrepareUIDrawCuledSourceLink::EventFunctionType)(&ModuleUpscaler::PrepareUIDrawCuledListener);

		return S_OK;
	}

	HRESULT ModuleUpscaler::ShutdownImpl()
	{
		// Imposible

		return S_FALSE;
	}
}