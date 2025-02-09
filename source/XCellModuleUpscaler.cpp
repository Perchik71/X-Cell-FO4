// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <d3d11.h>
#include <d3d11_2.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <comdef.h>
#include <ShlObj_core.h>

// F4SE
#include <f4se/PapyrusNativeFunctions.h>
#include <f4se/GameSettings.h>

// XCell
#include "XCellModuleUpscaler.h"
#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellPostProcessor.h"

#pragma comment(lib, "Comctl32.lib")

namespace XCell
{
	using namespace Microsoft::WRL;

	// Does not exist in Pref list
	static constexpr char MIPBIAS_OPTION_NAME[] = "fMipBias:Display";
	static constexpr char MAXANISTROPY_OPTION_NAME[] = "iMaxAnisotropy:Display";

	struct RenderWindow
	{
		HWND Handle = nullptr;
		UINT Width = 0;
		UINT Height = 0;
		UINT SourceWidth = 0;
		UINT SourceHeight = 0;
		ID3D11Device2* Device = nullptr;
		ID3D11DeviceContext2* DeviceContext = nullptr;
		IDXGISwapChain2* SwapChain = nullptr;
		D3D11_VIEWPORT SourceViewport;
	} gRenderWindow;
	
	decltype(&CreateWindowExA) gOldCreateWindow = 0;
	decltype(&GetClientRect) gOldGetClientRect = 0;
	decltype(&GetWindowRect) gOldGetWindowRect = 0;
	decltype(&SetWindowPos) gOldSetWindowPos = 0;


	UInt64 gOldD3DRSSetViewports = 0;
	UInt64 gOldSub14153D7D0 = 0;

	::Setting* gMipBiasSetting = nullptr;
	::Setting* gMaxAnisotropySetting = nullptr;

	static BOOL WINAPI HKSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
	{
		return TRUE;
	}

	static BOOL WINAPI HKGetClientRect(HWND hWnd, LPRECT pRect)
	{
		auto bRet = gOldGetClientRect(hWnd, pRect);

		if (bRet && (gRenderWindow.Handle == hWnd))
		{
			pRect->right = gRenderWindow.SourceWidth;
			pRect->bottom = gRenderWindow.SourceHeight;
		}

		return bRet;
	}

	static BOOL WINAPI HKGetWindowRect(HWND hWnd, LPRECT pRect)
	{
		auto bRet = gOldGetWindowRect(hWnd, pRect);

		if (bRet && (gRenderWindow.Handle == hWnd))
		{
			pRect->right = gRenderWindow.SourceWidth + pRect->left;
			pRect->bottom = gRenderWindow.SourceHeight + pRect->top;
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

			gRenderWindow.SourceWidth = nWidth;
			gRenderWindow.SourceHeight = nHeight;
			gRenderWindow.Width = gRenderWindow.SourceWidth / SettingScale;
			gRenderWindow.Height = gRenderWindow.SourceHeight / SettingScale;
			gRenderWindow.Handle = gOldCreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle, 
				X, Y, gRenderWindow.Width, gRenderWindow.Height, hWndParent, hMenu, hInstance, lpParam);
			return gRenderWindow.Handle;
		}

		return gOldCreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle,
			X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	static UInt64 WINAPI HKSub14153D7D0(UInt64 Unk0, UInt64 Unk1, UInt64 Unk2, UInt64 Unk3)
	{
		auto SettingSizeW = (::Setting*)REL::ID(230);
		auto SettingSizeH = (::Setting*)REL::ID(231);
		auto SettingScale = CVarDisplayScale->GetFloat();

		// Insurance in case of a repeat call
		if (!gRenderWindow.Handle)
		{
			_MESSAGE("Upscale: Settings loaded %ux%u scale %.3f", 
				SettingSizeW->data.u32, SettingSizeH->data.u32, SettingScale);

			SettingSizeW->data.u32 = (UInt32)(SettingSizeW->data.u32 * SettingScale);
			SettingSizeH->data.u32 = (UInt32)(SettingSizeH->data.u32 * SettingScale);

			_MESSAGE("Upscale: Changing the display settings %ux%u according to scale %.3f", 
				SettingSizeW->data.u32, SettingSizeH->data.u32, SettingScale);
		}

		return XCFastCall<UInt64>(gOldSub14153D7D0, Unk0, Unk1, Unk2, Unk3);
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

		auto hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, TestFeatureLevels,
			_ARRAYSIZE(TestFeatureLevels), D3D11_SDK_VERSION, ppDevice, &FeatureLevel, ppImmediateContext);
		if (FAILED(hr))
		{
			_ERROR("D3D11CreateDevice return failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			return E_FAIL;
		}

		_MESSAGE("Created D3D11 device with feature level %X...", FeatureLevel);

		hr = (*ppDevice)->QueryInterface<ID3D11Device2>(&gRenderWindow.Device);
		if (FAILED(hr))
		{
			_ERROR("Initialization error DirectX 11.2 (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			return E_FAIL;
		}

		gRenderWindow.Device->GetImmediateContext2(&gRenderWindow.DeviceContext);

		ComPtr<IDXGIFactory2> Factory;
		hr = CreateDXGIFactory2(0, IID_IDXGIFactory2, (void**)Factory.GetAddressOf());
		if (FAILED(hr))
		{
			_MESSAGE("Failed to create IDXGIFactory2 0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			return E_FAIL;
		}

		DXGI_SWAP_CHAIN_DESC1 scDesc;
		ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

		scDesc.Width = static_cast<UINT>(gRenderWindow.SourceWidth);
		scDesc.Height = static_cast<UINT>(gRenderWindow.SourceHeight);
		scDesc.Format = pSwapChainDesc->BufferDesc.Format;
		scDesc.Stereo = false;
		scDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = pSwapChainDesc->BufferUsage;
		scDesc.BufferCount = pSwapChainDesc->BufferCount;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scDesc.Scaling = DXGI_SCALING_STRETCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC scFullScreenDesc;
		ZeroMemory(&scFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

		scFullScreenDesc.RefreshRate.Numerator = (UINT)CVarMaxFrameRateVSync->GetUnsignedInt();
		scFullScreenDesc.RefreshRate.Denominator = (UINT)CVarVSync->GetBool();
		scFullScreenDesc.Windowed = pSwapChainDesc->Windowed;
		scFullScreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;

		_MESSAGE("Render Window: (0x%016llX) %ux%u", (UInt64)gRenderWindow.Handle, gRenderWindow.Width, gRenderWindow.Height);
		
		hr = Factory->CreateSwapChainForHwnd(gRenderWindow.Device, pSwapChainDesc->OutputWindow, &scDesc,
			&scFullScreenDesc, nullptr, (IDXGISwapChain1**)&gRenderWindow.SwapChain);
		if (FAILED(hr))
		{
			_MESSAGE("CreateSwapChainForHwnd returned failed (0x%08X): %s", hr, _com_error(hr).ErrorMessage());
			return E_FAIL;
		}

		// Change the region of the swap chain that will be presented to the screen
		gRenderWindow.SwapChain->SetSourceSize(gRenderWindow.SourceWidth, gRenderWindow.SourceHeight);

		_MESSAGE("HKD3D11CreateDeviceAndSwapChain: Successed");

		if (pFeatureLevel)
			*pFeatureLevel = FeatureLevel;

		*ppSwapChain = gRenderWindow.SwapChain;
		*ppDevice = gRenderWindow.Device;
		*ppImmediateContext = gRenderWindow.DeviceContext;

		return S_OK;
	}

	static FARPROC WINAPI HKGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{
		if (!strcmp(lpProcName, "D3D11CreateDeviceAndSwapChain"))
			// This is our client, it's time to deceive him
			return reinterpret_cast<FARPROC>(&HKD3D11CreateDeviceAndSwapChain);
		return GetProcAddress(hModule, lpProcName);
	}

	namespace PVM
	{
		static float GetMipLODBias(StaticFunctionTag* base)
		{
			return CVarLodMipBias->GetFloat();
		}

		static void SetMipLODBias(StaticFunctionTag* base, float value)
		{
			value = min(5.0f, max(-5.0f, value));

			_MESSAGE("MIP LOD Bias changed from %f to %f, recreating samplers", CVarLodMipBias->GetFloat(), value);

			GlobalPostProcessor->Reset();
			CVarLodMipBias->SetFloat(value);

			Game::XCWriteINISettingGameFloat(Game::gINIGameSettingPref, MIPBIAS_OPTION_NAME, value);
		}

		static void SetDefaultMipLODBias(StaticFunctionTag* base)
		{
			SetMipLODBias(base, 0.0f);
		}

		static float GetMaxAnisotropy(StaticFunctionTag* base)
		{
			return CVarMaxAnisotropy->GetUnsignedInt();
		}

		static void SetMaxAnisotropy(StaticFunctionTag* base, long value)
		{
			value = min(16l, max(0l, value));

			_MESSAGE("MAX Anisotropy changed from %u to %u, recreating samplers", CVarLodMipBias->GetUnsignedInt(), (UInt32)value);

			GlobalPostProcessor->Reset();
			CVarMaxAnisotropy->SetUnsignedInt((UInt32)value);

			Game::XCWriteINISettingGameInt(Game::gINIGameSettingPref, MAXANISTROPY_OPTION_NAME, value);
		}

		static void SetDefaultMaxAnisotropy(StaticFunctionTag* base)
		{
			SetMaxAnisotropy(base, 0);
		}
	}

	XCellModuleUpscaler::XCellModuleUpscaler(void* Context) :
		Module(Context, SourceName, CVarUpscaler)
	{
		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&XCellModuleUpscaler::DXListener);
		InitializePapyrusLinker.OnListener = (EventInitializePapyrusSourceLink::EventFunctionType)(&XCellModuleUpscaler::VMListener);
		RenderEndFrameLinker.OnListener = (EventRenderEndFrameSourceLink::EventFunctionType)(&XCellModuleUpscaler::DXEndFrameListener);
	}

	HRESULT XCellModuleUpscaler::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		if (GlobalPostProcessor)
		{
			delete GlobalPostProcessor;
			GlobalPostProcessor = nullptr;
		}

		auto Prefs = *g_iniPrefSettings.GetPtr();
		if (!Prefs)
			goto ReadINISettingItself;

		gMipBiasSetting = Prefs->Get(MIPBIAS_OPTION_NAME);
		gMaxAnisotropySetting = Prefs->Get(MAXANISTROPY_OPTION_NAME);
		if (!gMipBiasSetting || !gMaxAnisotropySetting)
		{
		ReadINISettingItself:
			long v;
			if (!Game::XCReadINISettingGameInt(Game::gINIGameSettingPref, MAXANISTROPY_OPTION_NAME, 0, &v))
				_WARNING("Error read INI setting \"%s\"", MAXANISTROPY_OPTION_NAME);
			else
				CVarMaxAnisotropy->SetUnsignedInt((UInt32)min(16l, max(0l, v)));
		}
		else
		{
			CVarLodMipBias->SetFloat(min(5.0f, max(-5.0f, gMipBiasSetting->data.f32)));
			CVarMaxAnisotropy->SetUnsignedInt(min(16ul, max(0ul, gMaxAnisotropySetting->data.u32)));
		}

		float v;
		Game::XCReadINISettingGameFloat(Game::gINIGameSettingPref, MIPBIAS_OPTION_NAME, 0.0f, &v);
		CVarLodMipBias->SetFloat(min(5.0f, max(-5.0f, v)));

		GlobalPostProcessor = new PostProcessor(Device, Context, SwapChain, WindowHandle);
		GlobalPostProcessor->Install();

		return S_OK;
	}

	HRESULT XCellModuleUpscaler::VMListener(VirtualMachine* VM)
	{
		VM->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, float>("GetMipLODBias", "XCELL", PVM::GetMipLODBias, VM));
		VM->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, void, float>("SetMipLODBias", "XCELL", PVM::SetMipLODBias, VM));
		VM->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, void>("SetDefaultMipLODBias", "XCELL", PVM::SetDefaultMipLODBias, VM));
		VM->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, float>("GetMaxAnisotropy", "XCELL", PVM::GetMaxAnisotropy, VM));
		VM->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, void, long>("SetMaxAnisotropy", "XCELL", PVM::SetMaxAnisotropy, VM));
		VM->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, void>("SetDefaultMaxAnisotropy", "XCELL", PVM::SetDefaultMaxAnisotropy, VM));

		return S_OK;
	}

	HRESULT XCellModuleUpscaler::DXEndFrameListener()
	{
		// In Direct3D, change the Viewport to match the region of the swap
		// chain that will now be presented from.
		//gD3D11DeviceContext2->RSSetViewports(1, &RenderViewport);

		GlobalPostProcessor->PrepareUpscaler();

		return S_OK;
	}

	HRESULT XCellModuleUpscaler::InstallImpl()
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
			{
				// If we fail, then it's not ENB or Boris has become arrogant
				return E_FAIL;

				// unsafe

				//SetDllDirectoryA("");
				//strcat_s(szSystemPath, "\\d3d11.dll");
				//auto hD3D11ModuleOrig = LoadLibraryA(szSystemPath);
				//SetDllDirectoryA(NULL);

				//if (!hD3D11ModuleOrig)
				//{
				//	_ERROR("Module \"%s\" not found", szSystemPath);
				//	return E_FAIL;
				//}		

				//_MESSAGE("Address module \"d3d11.dll\": %p", (void*)hD3D11ModuleOrig);

				//auto D3D11CreateDeviceAndSwapChainPtr = (UInt64)GetProcAddress(hD3D11ModuleOrig, "D3D11CreateDeviceAndSwapChain");
				//if (!D3D11CreateDeviceAndSwapChainPtr)
				//{
				//	_ERROR("Function \"D3D11CreateDeviceAndSwapChain\" not found in \"d3d11.dll\"");
				//	return E_FAIL;
				//}

				//// mov rax, HKD3D11CreateDeviceAndSwapChain
				//// jmp rax
				//// nop
				//auto Address = (UInt64)&HKD3D11CreateDeviceAndSwapChain;
				//REL::Impl::Patch(D3D11CreateDeviceAndSwapChainPtr, { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				//	0xFF, 0xE0, 0x90 });
				//REL::Impl::PatchT(D3D11CreateDeviceAndSwapChainPtr + 2, Address);
			}


			//REL::Impl::PatchNop(REL::ID(230), 6);
			//REL::Impl::DetourCall(REL::ID(230), (UInt64)&HKD3DRSSetViewports);

			
			//REL::Impl::DetourCall(REL::ID(231), (UInt64)&HKD3DRSSetViewports);
		}
		else
		{
			// A person does not use ENB, other nonsense
			REL::Impl::DetourIAT(
				pvContext->ProcessBase,
				"d3d11.dll", 
				"D3D11CreateDeviceAndSwapChain", 
				(UInt64)&HKD3D11CreateDeviceAndSwapChain);
		}

		// I'll intercept this function, at this stage, the settings have already been loaded, but they have not yet been accepted.
		gOldSub14153D7D0 = REL::Impl::DetourJump(REL::ID(240), (UInt64)&HKSub14153D7D0);	
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

		return S_OK;
	}

	HRESULT XCellModuleUpscaler::ShutdownImpl()
	{
		// Imposible

		return S_FALSE;
	}
}