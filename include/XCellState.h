// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace XCell
{
	using namespace Microsoft::WRL;
	
	// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h#L22
	class State
	{
		ComPtr<ID3D11VertexShader> VertexShader;
		ComPtr<ID3D11PixelShader> PixelShader;
		ComPtr<ID3D11ComputeShader> ComputeShader;
		ComPtr<ID3D11InputLayout> InputLayout;
		D3D11_PRIMITIVE_TOPOLOGY Topology;
		ID3D11Buffer* VertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		UINT Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		UINT Offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		ComPtr<ID3D11Buffer> IndexBuffer;
		DXGI_FORMAT Format;
		UINT Offset;
		ID3D11RenderTargetView* RenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ComPtr<ID3D11DepthStencilView> DepthStencil;
		ComPtr<ID3D11RasterizerState> RasterizerState;
		ComPtr<ID3D11DepthStencilState> DepthStencilState;
		UINT StencilRef;
		D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		UINT NumViewports = 0;
		ComPtr<ID3D11Buffer> VSConstantBuffer;
		ComPtr<ID3D11Buffer> PSConstantBuffer;
		ComPtr<ID3D11Buffer> CSConstantBuffer;
		UINT NumScissorRects;
		D3D11_RECT RSScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ComPtr<ID3D11RasterizerState> RS;
		ComPtr<ID3D11BlendState> BlendState;
		UINT SampleMask;
		FLOAT BlendFactor[4];
		ID3D11ShaderResourceView* CSShaderResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11UnorderedAccessView* CSUavs[D3D11_1_UAV_SLOT_COUNT];
		ID3D11DeviceContext* DeviceContext;
	public:
		State(ID3D11DeviceContext* DeviceContext);
		virtual ~State() = default;

		virtual void PushState() noexcept(true);
		virtual void PopState() noexcept(true);

		State(const State&) = delete;
		State& operator=(const State&) = delete;
	};
}