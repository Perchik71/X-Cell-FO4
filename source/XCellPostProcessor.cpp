// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellPostProcessor.h"
#include "XCellRelocator.h"
#include "XCellCVar.h"

namespace XCell
{
	PostProcessor* GlobalPostProcessor = nullptr;

	static void __stdcall HKPSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(0, This, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKVSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(1, This, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKGSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(2, This, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKHSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(3, This, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKDSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(4, This, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKCSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(5, This, StartSlot, NumSamplers, samplers);
	}

	PostProcessor::PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain) :
		_D3D11Device(D3D11Device), _D3D11DeviceContext(D3D11DeviceContext), _DXGISwapChain(DXGISwapChain)
	{}

	// Mostly from vrperfkit, thanks to fholger for showing how to do mip lod bias
	// https://github.com/fholger/vrperfkit/blob/037c09f3168ac045b5775e8d1a0c8ac982b5854f/src/d3d11/d3d11_post_processor.cpp#L76
	void PostProcessor::PreXSSetSamplers(ID3D11SamplerState** OutSamplers, UINT StartSlot, UINT NumSamplers, 
		ID3D11SamplerState* const* Samplers)
	{
		memcpy(OutSamplers, Samplers, NumSamplers * sizeof(ID3D11SamplerState*));
		for (UINT i = 0; i < NumSamplers; ++i)
		{
			auto orig = OutSamplers[i];
			if (orig == nullptr || PassThroughSamplers.find(orig) != PassThroughSamplers.end())
				continue;

			if (MappedSamplers.find(orig) == MappedSamplers.end())
			{
				D3D11_SAMPLER_DESC sd;
				orig->GetDesc(&sd);

				if (sd.MipLODBias)
				{
					// do not mess with samplers that already have a bias or are not doing anisotropic filtering.
					// should hopefully reduce the chance of causing rendering errors.
					PassThroughSamplers.insert(orig);
					continue;
				}
				
				// _MESSAGE("Filter: %u, MipLODBias: %f, MaxAnisotropy: %u", (UInt32)sd.Filter, sd.MipLODBias, sd.MaxAnisotropy);

				sd.MipLODBias = CVarLodMipBias->GetFloat();
				sd.MaxAnisotropy = (sd.Filter == D3D11_FILTER_ANISOTROPIC) ? CVarMaxAnisotropy->GetUnsignedInt() : 0;
				sd.MinLOD = 0;
				sd.MaxLOD = D3D11_FLOAT32_MAX;

				_D3D11Device->CreateSamplerState(&sd, MappedSamplers[orig].GetAddressOf());
				PassThroughSamplers.insert(MappedSamplers[orig].Get());
			}

			OutSamplers[i] = MappedSamplers[orig].Get();
		}
	}

	void PostProcessor::NativeCallXSSetSamplers(UInt8 Index, ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers,
		ID3D11SamplerState* const* Samplers)
	{
		XCFastCall<void>(_OldFunctions[Index], This, StartSlot, NumSamplers, Samplers);
	}

	void PostProcessor::Install()
	{
		_OldFunctions[0] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKPSSetSamplers, 10);
		_OldFunctions[1] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKVSSetSamplers, 26);
		_OldFunctions[2] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKGSSetSamplers, 32);
		_OldFunctions[3] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKHSSetSamplers, 61);
		_OldFunctions[4] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKDSSetSamplers, 65);
		_OldFunctions[5] = REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), (UInt64)&HKCSSetSamplers, 70);
	}

	void PostProcessor::Shutdown()
	{
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[0], 10);
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[1], 26);
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[2], 32);
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[3], 61);
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[4], 65);
		REL::Impl::DetourVTable(*((UInt64*)_D3D11DeviceContext), _OldFunctions[5], 70);

		Reset();
	}

	void PostProcessor::Reset()
	{
		PassThroughSamplers.clear();
		MappedSamplers.clear();
	}
}