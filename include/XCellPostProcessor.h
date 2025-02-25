// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <ICriticalSection.h>
#include <f4se/GameMenus.h>

#include "XCellPixelShader.h"
#include "XCellVertexShader.h"
#include "XCellComputeShader.h"
#include "XCellClassesShader.h"
#include "XCellTAA.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace XCell
{
	using namespace Microsoft::WRL;

	class PostProcessor;
	class RenderFrame : public Object
	{
		const PostProcessor* _core;
		unique_ptr<TextureShader> _colorBuffer;
		unique_ptr<TextureShader> _depthBuffer;
		unique_ptr<RenderTargetView> _renderTarget;
		unique_ptr<ResourceView> _colorView;
		unique_ptr<ResourceView> _depthView;
	public:
		RenderFrame(const char* Name, const PostProcessor* Core);
		virtual ~RenderFrame() = default;

		RenderFrame(const RenderFrame&) = delete;
		RenderFrame& operator=(const RenderFrame&) = delete;

		virtual bool Initialize(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true);
		virtual bool CopyFrame(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true);
		virtual bool CopyFrame(const RenderFrame* Frame) noexcept(true);
		virtual void DebugSaveFrameToFiles() const noexcept(true);
		virtual void DebugInfo() const noexcept(true);

		virtual inline void InitColorPipeline(UInt32 Type, UInt32 BindID) const noexcept(true)
		{ _colorView->InitPipeline(Type, BindID); }
		virtual inline void InitDepthPipeline(UInt32 Type, UInt32 BindID) const noexcept(true) 
		{ _depthView->InitPipeline(Type, BindID); }

		virtual inline void ShutdownColorPipeline() const noexcept(true)
		{ _colorView->ShutdownPipeline(); }
		virtual inline void ShutdownDepthPipeline() const noexcept(true)
		{ _depthView->ShutdownPipeline(); }

		[[nodiscard]] virtual inline ID3D11Resource* GetColor() const noexcept(true) { return _colorBuffer->Get(); }
		[[nodiscard]] virtual inline ID3D11Resource* GetDepth() const noexcept(true) { return _depthBuffer->Get(); }
		[[nodiscard]] virtual inline ID3D11ShaderResourceView* GetColorView() const noexcept(true) { return _colorView->Get(); }
		[[nodiscard]] virtual inline ID3D11ShaderResourceView* GetDepthView() const noexcept(true) { return _depthView->Get(); }
	};

	class HistoryFrames : public Object
	{
		const PostProcessor* _core;
		UINT64 _count;
		ICriticalSection _CriticalSection;
		unique_ptr<RenderFrame> _currFrame, _prevFrame;
	public:
		HistoryFrames(const char* Name, const PostProcessor* Core);
		virtual ~HistoryFrames() = default;

		HistoryFrames(const HistoryFrames&) = delete;
		HistoryFrames& operator=(const HistoryFrames&) = delete;

		virtual void StoreFrame(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true);
		virtual void ClearHistory() noexcept(true);
		virtual void DebugSaveCurrentFrameToFiles() const noexcept(true);

		[[nodiscard]] virtual inline UINT64 Count() const noexcept(true) { return _count; }
		[[nodiscard]] virtual inline const RenderFrame* CurrentFrame() const noexcept(true) { return _currFrame.get(); }
		[[nodiscard]] virtual inline const RenderFrame* PreviousFrame() const noexcept(true) { return _prevFrame.get(); }
	};

	class PostProcessor
	{
		FLOAT _Width, _Height;
		UINT64 _CountFrame;
		HWND _RenderWindow;
		ID3D11Device* _D3D11Device;
		ID3D11DeviceContext* _D3D11DeviceContext;
		IDXGISwapChain* _DXGISwapChain;	
		ComPtr<ID3D11RasterizerState> _D3D11RasterizerState;		
		unique_ptr<HistoryFrames> _history;
		unique_ptr<TAA> _taa;
	public:
		PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain,
			HWND RenderWindow);

		virtual inline void GetWindowSize(FLOAT* Width, FLOAT* Height) const noexcept(true) { *Width = _Width; *Height = _Height; }
		virtual inline FLOAT GetWindowWidth() const noexcept(true) { return _Width; }
		virtual inline FLOAT GetWindowHeight() const noexcept(true) { return _Height; }
		XCPropertyReadOnly(GetWindowWidth) FLOAT WindowWidth;
		XCPropertyReadOnly(GetWindowHeight) FLOAT WindowHeight;

		virtual inline HWND GetWindowHandle() const noexcept(true) { return _RenderWindow; }
		XCPropertyReadOnly(GetWindowHandle) HWND WindowHandle;

		virtual inline ID3D11Device* GetDevice() const noexcept(true) { return _D3D11Device; }
		virtual inline ID3D11DeviceContext* GetDeviceContext() const noexcept(true) { return _D3D11DeviceContext; }
		virtual inline IDXGISwapChain* GetSwapChain() const noexcept(true) { return _DXGISwapChain; }
		XCPropertyReadOnly(GetDevice) ID3D11Device* Device;
		XCPropertyReadOnly(GetDeviceContext) ID3D11DeviceContext* DeviceContext;
		XCPropertyReadOnly(GetSwapChain) IDXGISwapChain* SwapChain;

		virtual inline const HistoryFrames* GetHistoryFrames() const noexcept(true) { return _history.get(); }
		XCPropertyReadOnly(GetHistoryFrames) const HistoryFrames* History;

		// METHODS
		virtual void Install();
		virtual void Shutdown();
		virtual void Processing(ID3D11RenderTargetView* pRenderTarget, ID3D11Resource* pBackBuffer, 
			ID3D11Resource* pDepthStencilBuffer) noexcept(true);
	} extern *GlobalPostProcessor;
}