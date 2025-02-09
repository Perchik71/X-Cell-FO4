// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include <unordered_map>
#include <unordered_set>

namespace XCell
{
	using namespace Microsoft::WRL;

	// https://github.com/fholger/vrperfkit/blob/a52f8a45d330d0b66206aee85165db715e4482cd/src/d3d11/d3d11_helper.h#L22
	struct PostProcessorState 
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
		ID3D11ShaderResourceView* CSShaderResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11UnorderedAccessView* CSUavs[D3D11_1_UAV_SLOT_COUNT];
	};

	struct PostProcessorInput
	{
		UINT NumInputViewports;
		D3D11_VIEWPORT InputViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11Texture2D* InputTexture;		
		ComPtr<ID3D11Texture2D> OutputTexture;
		D3D11_TEXTURE2D_DESC InputTextureDesc;
		D3D11_TEXTURE2D_DESC OutputTextureDesc;
	};

	//struct PostProcessorInput
	//{
	//	
	//	
	//	ID3D11ShaderResourceView* InputView;
	//	ID3D11ShaderResourceView* OutputView;
	//	ID3D11UnorderedAccessView* OutputUav;
	//};

	class PostProcessor
	{
		FLOAT _Width, _Height;
		HWND _RenderWindow;
		ID3D11Device* _D3D11Device;
		ID3D11DeviceContext* _D3D11DeviceContext;
		IDXGISwapChain* _DXGISwapChain;
		D3D11_VIEWPORT _D3D11Viewport;
		UInt64 _OldFunctions[8];
		PostProcessorState _State;
		PostProcessorInput _Input;

		ID3D11RenderTargetView* _render;

		bool _StateStared;
		bool _Failed;

		unordered_set<ID3D11SamplerState*> PassThroughSamplers;
		unordered_map<ID3D11SamplerState*, ComPtr<ID3D11SamplerState>> MappedSamplers;

		virtual void PushState() noexcept(true);
		virtual void PopState() noexcept(true);
	public:
		PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain,
			HWND RenderWindow);

		inline virtual void GetWindowSize(FLOAT* Width, FLOAT* Height) const noexcept(true) { *Width = _Width; *Height = _Height; }
		inline virtual FLOAT GetWindowWidth() const noexcept(true) { return _Width; }
		inline virtual FLOAT GetWindowHeight() const noexcept(true) { return _Height; }	
		XCPropertyReadOnly(GetWindowWidth) FLOAT WindowWidth;
		XCPropertyReadOnly(GetWindowHeight) FLOAT WindowHeight;

		inline virtual HWND GetWindowHandle() const noexcept(true) { return _RenderWindow; }
		XCPropertyReadOnly(GetWindowHandle) HWND WindowHandle;

		inline virtual ID3D11Device* GetDevice() const noexcept(true) { return _D3D11Device; }
		inline virtual ID3D11DeviceContext* GetDeviceContext() const noexcept(true) { return _D3D11DeviceContext; }
		inline virtual IDXGISwapChain* GetSwapChain() const noexcept(true) { return _DXGISwapChain; }
		XCPropertyReadOnly(GetDevice) ID3D11Device* Device;
		XCPropertyReadOnly(GetDeviceContext) ID3D11DeviceContext* DeviceContext;
		XCPropertyReadOnly(GetSwapChain) IDXGISwapChain* SwapChain;

		// IMPLS
		virtual void PreXSSetSamplers(ID3D11SamplerState** OutSamplers, UINT StartSlot, UINT NumSamplers,
			ID3D11SamplerState* const* Samplers);
		virtual void NativeCallXSSetSamplers(UInt8 Index, UINT StartSlot, UINT NumSamplers,
			ID3D11SamplerState* const* Samplers);
		virtual void NativeCallClearRenderTargetView(ID3D11RenderTargetView* RenderTargetView, const FLOAT* ColorRGBA);

		// METHODS
		virtual void Install();
		virtual void Shutdown();
		virtual void Reset();
		virtual void PrepareUpscaler() noexcept(true);
	} extern *GlobalPostProcessor;
}