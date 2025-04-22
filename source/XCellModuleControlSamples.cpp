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
#include "XCellModuleControlSamples.h"
#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"

namespace XCell
{
	using namespace Microsoft::WRL;

	// Does not exist in Pref list
	static constexpr char MIPBIAS_OPTION_NAME[] = "fMipBias:Display";
	static constexpr char MAXANISTROPY_OPTION_NAME[] = "iMaxAnisotropy:Display";

	::Setting* gMipBiasSetting = nullptr;
	::Setting* gMaxAnisotropySetting = nullptr;
	ModuleControlSamples* gModuleControlSamples = nullptr;

	///////////////////////////////////////////////////////////////////////////////

	bool CheckAddrState(ID3D11SamplerState* Ptr)
	{
		__try
		{
			D3D11_SAMPLER_DESC sd;
			Ptr->GetDesc(&sd);
			return true;
		}
		__except(1)
		{
			return false;
		}
	}

	///////////////////////////////////////////////////////////////////////////////

	// Mostly from vrperfkit, thanks to fholger for showing how to do mip lod bias
	// https://github.com/fholger/vrperfkit/blob/037c09f3168ac045b5775e8d1a0c8ac982b5854f/src/d3d11/d3d11_post_processor.cpp#L76
	void ModuleControlSamples::PreXSSetSamplers(ID3D11SamplerState** OutSamplers, UINT StartSlot, UINT NumSamplers,
		ID3D11SamplerState* const* Samplers)
	{
		memcpy(OutSamplers, Samplers, NumSamplers * sizeof(ID3D11SamplerState*));
		for (UINT i = 0; i < NumSamplers; ++i)
		{
			auto orig = OutSamplers[i];
			if ((orig == nullptr) || (PassThroughSamplers.find(orig) != PassThroughSamplers.end()) || !CheckAddrState(orig))
				continue;

			if (MappedSamplers.find(orig) == MappedSamplers.end())
			{
				D3D11_SAMPLER_DESC sd;
				orig->GetDesc(&sd);

				if (sd.MipLODBias)
				{
					// do not mess with samplers that already have a bias.
					// should hopefully reduce the chance of causing rendering errors.
					PassThroughSamplers.insert(orig);
					continue;
				}

				// _MESSAGE("Filter: %u, MipLODBias: %f, MaxAnisotropy: %u", (UInt32)sd.Filter, sd.MipLODBias, sd.MaxAnisotropy);

				if (sd.Filter != D3D11_FILTER_ANISOTROPIC) 
					sd.MipLODBias = CVarLodMipBias->GetFloat();

				sd.MaxAnisotropy = (sd.Filter == D3D11_FILTER_ANISOTROPIC) ? CVarMaxAnisotropy->GetUnsignedInt() : 0;
				sd.MinLOD = 0;
				sd.MaxLOD = D3D11_FLOAT32_MAX;

				Device->CreateSamplerState(&sd, MappedSamplers[orig].GetAddressOf());
				PassThroughSamplers.insert(MappedSamplers[orig].Get());
			}

			OutSamplers[i] = MappedSamplers[orig].Get();
		}
	}

	void ModuleControlSamples::NativeCallXSSetSamplers(UInt8 Index, UINT StartSlot, UINT NumSamplers,
		ID3D11SamplerState* const* Samplers)
	{
		XCFastCall<void>(_OldFunctions[Index], DeviceContext, StartSlot, NumSamplers, Samplers);
	}

	static void __stdcall HKPSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(0, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKVSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(1, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKGSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(2, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKHSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(3, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKDSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(4, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKCSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		gModuleControlSamples->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		gModuleControlSamples->NativeCallXSSetSamplers(5, StartSlot, NumSamplers, samplers);
	}

	///////////////////////////////////////////////////////////////////////////////

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

			gModuleControlSamples->Reset();
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

			gModuleControlSamples->Reset();
			CVarMaxAnisotropy->SetUnsignedInt((UInt32)value);

			Game::XCWriteINISettingGameInt(Game::gINIGameSettingPref, MAXANISTROPY_OPTION_NAME, value);
		}

		static void SetDefaultMaxAnisotropy(StaticFunctionTag* base)
		{
			SetMaxAnisotropy(base, 0);
		}
	}

	ModuleControlSamples::ModuleControlSamples(void* Context) :
		Module(Context, SourceName, XCELL_MODULE_QUERY_DIRECTX_INIT | XCELL_MODULE_QUERY_PAPYRUS_INIT),
		Device(nullptr), DeviceContext(nullptr)
	{
		gModuleControlSamples = this;
		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&ModuleControlSamples::DXListener);
		InitializePapyrusLinker.OnListener = (EventInitializePapyrusSourceLink::EventFunctionType)(&ModuleControlSamples::VMListener);
	}

	HRESULT ModuleControlSamples::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		this->Device = Device;
		this->DeviceContext = Context;

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

		_OldFunctions[0] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKPSSetSamplers, 10);
		_OldFunctions[1] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKVSSetSamplers, 26);
		_OldFunctions[2] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKGSSetSamplers, 32);
		_OldFunctions[3] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKHSSetSamplers, 61);
		_OldFunctions[4] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKDSSetSamplers, 65);
		_OldFunctions[5] = REL::Impl::DetourVTable(*((UInt64*)DeviceContext), (UInt64)&HKCSSetSamplers, 70);

		return S_OK;
	}

	HRESULT ModuleControlSamples::VMListener(VirtualMachine* VM)
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

	HRESULT ModuleControlSamples::InstallImpl()
	{
		CVarLodMipBias->SetFloat(0);
		CVarMaxAnisotropy->SetUnsignedInt(0);

		return S_OK;
	}

	void ModuleControlSamples::Reset()
	{
		PassThroughSamplers.clear();
		MappedSamplers.clear();
	}

	HRESULT ModuleControlSamples::ShutdownImpl()
	{
		if (DeviceContext)
		{
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[0], 10);
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[1], 26);
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[2], 32);
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[3], 61);
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[4], 65);
			REL::Impl::DetourVTable(*((UInt64*)DeviceContext), _OldFunctions[5], 70);
		}

		Reset();

		return S_OK;
	}
}