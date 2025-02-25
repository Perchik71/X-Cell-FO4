// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellState.h"

namespace XCell
{
	State::State(ID3D11DeviceContext* DeviceContext) :
		DeviceContext(DeviceContext)
	{}

	void State::PushState() noexcept(true)
	{
		// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h

		DeviceContext->VSGetShader(VertexShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		DeviceContext->PSGetShader(PixelShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		DeviceContext->CSGetShader(ComputeShader.ReleaseAndGetAddressOf(), nullptr, nullptr);
		DeviceContext->IAGetInputLayout(InputLayout.ReleaseAndGetAddressOf());
		DeviceContext->IAGetPrimitiveTopology(&Topology);
		DeviceContext->IAGetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, VertexBuffers, Strides, Offsets);
		DeviceContext->IAGetIndexBuffer(IndexBuffer.ReleaseAndGetAddressOf(), &Format, &Offset);
		DeviceContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, RenderTargets, DepthStencil.GetAddressOf());
		DeviceContext->RSGetState(RasterizerState.ReleaseAndGetAddressOf());
		DeviceContext->OMGetDepthStencilState(DepthStencilState.ReleaseAndGetAddressOf(), &StencilRef);
		DeviceContext->RSGetViewports(&NumViewports, nullptr);
		DeviceContext->RSGetViewports(&NumViewports, Viewports);
		DeviceContext->VSGetConstantBuffers(0, 1, VSConstantBuffer.GetAddressOf());
		DeviceContext->PSGetConstantBuffers(0, 1, PSConstantBuffer.GetAddressOf());
		DeviceContext->CSGetConstantBuffers(0, 1, CSConstantBuffer.GetAddressOf());
		DeviceContext->CSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, CSShaderResources);
		DeviceContext->CSGetUnorderedAccessViews(0, D3D11_1_UAV_SLOT_COUNT, CSUavs);
		
		// Added
		DeviceContext->RSGetScissorRects(&NumScissorRects, nullptr);
		DeviceContext->RSGetScissorRects(&NumScissorRects, RSScissorRects);
		DeviceContext->RSGetState(RS.GetAddressOf());
		DeviceContext->OMGetBlendState(BlendState.GetAddressOf(), BlendFactor, &SampleMask);
	}

	void State::PopState() noexcept(true)
	{
		// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h

		DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
		DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
		DeviceContext->CSSetShader(ComputeShader.Get(), nullptr, 0);
		DeviceContext->IASetInputLayout(InputLayout.Get());
		DeviceContext->IASetPrimitiveTopology(Topology);
		DeviceContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, VertexBuffers, Strides, Offsets);

		for (int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
			if (VertexBuffers[i])
				VertexBuffers[i]->Release();

		DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), Format, Offset);
		DeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, RenderTargets, DepthStencil.Get());

		for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			if (RenderTargets[i])
				RenderTargets[i]->Release();

		DeviceContext->RSSetState(RasterizerState.Get());
		DeviceContext->OMSetDepthStencilState(DepthStencilState.Get(), StencilRef);
		DeviceContext->RSSetViewports(NumViewports, Viewports);
		DeviceContext->VSSetConstantBuffers(0, 1, VSConstantBuffer.GetAddressOf());
		DeviceContext->PSSetConstantBuffers(0, 1, PSConstantBuffer.GetAddressOf());
		DeviceContext->CSSetConstantBuffers(0, 1, CSConstantBuffer.GetAddressOf());
		DeviceContext->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, CSShaderResources);

		for (int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
			if (CSShaderResources[i])
				CSShaderResources[i]->Release();

		UINT initial = 0;
		DeviceContext->CSSetUnorderedAccessViews(0, D3D11_1_UAV_SLOT_COUNT, CSUavs, &initial);

		for (int i = 0; i < D3D11_1_UAV_SLOT_COUNT; ++i)
			if (CSUavs[i])
				CSUavs[i]->Release();

		// Added
		DeviceContext->RSSetScissorRects(NumScissorRects, RSScissorRects);
		DeviceContext->RSSetState(RS.Get());
		DeviceContext->OMSetBlendState(BlendState.Get(), BlendFactor, SampleMask);
	}
}