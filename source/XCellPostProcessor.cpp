// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellPostProcessor.h"
#include "XCellRelocator.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

namespace XCell
{
	PostProcessor* GlobalPostProcessor = nullptr;

	static void __stdcall HKPSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(0, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKVSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(1, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKGSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(2, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKHSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(3, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKDSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(4, StartSlot, NumSamplers, samplers);
	}

	static void __stdcall HKCSSetSamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* Samplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		GlobalPostProcessor->PreXSSetSamplers(samplers, StartSlot, NumSamplers, Samplers);
		GlobalPostProcessor->NativeCallXSSetSamplers(5, StartSlot, NumSamplers, samplers);
	}

	PostProcessor::PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain,
		HWND RenderWindow) :
		_D3D11Device(D3D11Device), _D3D11DeviceContext(D3D11DeviceContext), _DXGISwapChain(DXGISwapChain), _RenderWindow(RenderWindow),
		_Width(0), _Height(0), _StateStared(false), _Failed(false)
	{}

	void PostProcessor::PushState() noexcept(true)
	{
		if (_StateStared) return;

		// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h

		_D3D11DeviceContext->VSGetShader(_State.VertexShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		_D3D11DeviceContext->PSGetShader(_State.PixelShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		_D3D11DeviceContext->CSGetShader(_State.ComputeShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		_D3D11DeviceContext->IAGetInputLayout(_State.InputLayout.ReleaseAndGetAddressOf());
		_D3D11DeviceContext->IAGetPrimitiveTopology(&_State.Topology);
		_D3D11DeviceContext->IAGetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, _State.VertexBuffers, _State.Strides, _State.Offsets);
		_D3D11DeviceContext->IAGetIndexBuffer(_State.IndexBuffer.ReleaseAndGetAddressOf(), &_State.Format, &_State.Offset);
		_D3D11DeviceContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, _State.RenderTargets, _State.DepthStencil.GetAddressOf());
		_D3D11DeviceContext->RSGetState(_State.RasterizerState.ReleaseAndGetAddressOf());
		_D3D11DeviceContext->OMGetDepthStencilState(_State.DepthStencilState.ReleaseAndGetAddressOf(), &_State.StencilRef);
		_D3D11DeviceContext->RSGetViewports(&_State.NumViewports, nullptr);
		_D3D11DeviceContext->RSGetViewports(&_State.NumViewports, _State.Viewports);
		_D3D11DeviceContext->VSGetConstantBuffers(0, 1, _State.VSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->PSGetConstantBuffers(0, 1, _State.PSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->CSGetConstantBuffers(0, 1, _State.CSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->CSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, _State.CSShaderResources);
		_D3D11DeviceContext->CSGetUnorderedAccessViews(0, D3D11_1_UAV_SLOT_COUNT, _State.CSUavs);
	}

	void PostProcessor::PopState() noexcept(true)
	{
		if (!_StateStared) return;

		// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h

		_D3D11DeviceContext->VSSetShader(_State.VertexShader.Get(), nullptr, 0);
		_D3D11DeviceContext->PSSetShader(_State.PixelShader.Get(), nullptr, 0);
		_D3D11DeviceContext->CSSetShader(_State.ComputeShader.Get(), nullptr, 0);
		_D3D11DeviceContext->IASetInputLayout(_State.InputLayout.Get());
		_D3D11DeviceContext->IASetPrimitiveTopology(_State.Topology);
		_D3D11DeviceContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, _State.VertexBuffers, _State.Strides, _State.Offsets);

		for (int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
			if (_State.VertexBuffers[i])
				_State.VertexBuffers[i]->Release();

		_D3D11DeviceContext->IASetIndexBuffer(_State.IndexBuffer.Get(), _State.Format, _State.Offset);
		_D3D11DeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, _State.RenderTargets, _State.DepthStencil.Get());

		for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			if (_State.RenderTargets[i])
				_State.RenderTargets[i]->Release();

		_D3D11DeviceContext->RSSetState(_State.RasterizerState.Get());
		_D3D11DeviceContext->OMSetDepthStencilState(_State.DepthStencilState.Get(), _State.StencilRef);
		_D3D11DeviceContext->RSSetViewports(_State.NumViewports, _State.Viewports);
		_D3D11DeviceContext->VSSetConstantBuffers(0, 1, _State.VSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->PSSetConstantBuffers(0, 1, _State.PSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->CSSetConstantBuffers(0, 1, _State.CSConstantBuffer.GetAddressOf());
		_D3D11DeviceContext->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, _State.CSShaderResources);

		for (int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
			if (_State.CSShaderResources[i])
				_State.CSShaderResources[i]->Release();

		UINT initial = 0;
		_D3D11DeviceContext->CSSetUnorderedAccessViews(0, D3D11_1_UAV_SLOT_COUNT, _State.CSUavs, &initial);

		for (int i = 0; i < D3D11_1_UAV_SLOT_COUNT; ++i)
			if (_State.CSUavs[i])
				_State.CSUavs[i]->Release();
	}

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

	void PostProcessor::NativeCallXSSetSamplers(UInt8 Index, UINT StartSlot, UINT NumSamplers,
		ID3D11SamplerState* const* Samplers)
	{
		XCFastCall<void>(_OldFunctions[Index], _D3D11DeviceContext, StartSlot, NumSamplers, Samplers);
	}

	void PostProcessor::NativeCallClearRenderTargetView(ID3D11RenderTargetView* RenderTargetView, const FLOAT* ColorRGBA)
	{
		XCFastCall<void>(_OldFunctions[7], _D3D11DeviceContext, RenderTargetView, ColorRGBA);
	}

	void PostProcessor::PrepareUpscaler() noexcept(true)
	{

	}

	void PostProcessor::Install()
	{
		if (IsWindow(_RenderWindow))
		{
			RECT rc;
			if (GetClientRect(_RenderWindow, &rc))
			{
				_Width = rc.right;
				_Height = rc.bottom;
			}
		}

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