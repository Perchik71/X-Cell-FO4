// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <d3d11.h>
#include <dxgi.h>

#include <f4se/PapyrusNativeFunctions.h>

#include "XCellModuleUpscaler.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellPostProcessor.h"

namespace XCell
{
	using namespace Microsoft::WRL;

	static UInt64 D3D11SamplerSubsPtr[6] = { 0 };
	static UInt64 DXGIPresent = 0;
	static ID3D11Device* D3D11UpscalerDevice = nullptr;


	


	

	static void __stdcall HKPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
	{
		XCFastCall<void>(DXGIPresent, This, SyncInterval, (UINT)CVarVSync->GetBool());
	}

	namespace PVM
	{
		static float GetMipLODBias(StaticFunctionTag* base)
		{
			return CVarLodMipBias->GetFloat();
		}

		static void SetMipLODBias(StaticFunctionTag* base, float value)
		{
			value = std::min(5.0f, std::max(-5.0f, value));

			_MESSAGE("MIP LOD Bias changed from %f to %f, recreating samplers", CVarLodMipBias->GetFloat(), value);

			GlobalPostProcessor->Reset();
			CVarLodMipBias->SetFloat(value);
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
			value = std::min(16l, std::max(0l, value));

			_MESSAGE("MAX Anisotropy changed from %u to %u, recreating samplers", CVarLodMipBias->GetUnsignedInt(), (UInt32)value);

			GlobalPostProcessor->Reset();
			CVarMaxAnisotropy->SetUnsignedInt((UInt32)value);
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
	}

	HRESULT XCellModuleUpscaler::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		D3D11UpscalerDevice = Device;

		if (GlobalPostProcessor)
		{
			delete GlobalPostProcessor;
			GlobalPostProcessor = nullptr;
		}

		GlobalPostProcessor = new PostProcessor(Device, Context, SwapChain);
		GlobalPostProcessor->Install();
		
		/**(UInt64*)&DXGIPresent = REL::Impl::DetourVTable(*(UInt64*)(SwapChain),
			(UInt64)&HKPresent, 8);*/

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

	HRESULT XCellModuleUpscaler::InstallImpl()
	{
		CVarLodMipBias->SetFloat(-1.3);
		CVarMaxAnisotropy->SetUnsignedInt(16);
		CVarVSync->SetBool(false);
		//PVM::SetMipLODBias(nullptr, g_plugin->read_setting_float("graphics", "miplodbias", -1.3f));

		return S_OK;
	}

	HRESULT XCellModuleUpscaler::ShutdownImpl()
	{
		// Imposible

		return S_FALSE;
	}
}