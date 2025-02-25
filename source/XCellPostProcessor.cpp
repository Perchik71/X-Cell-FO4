// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellCommonType.h"
#include "XCellPostProcessor.h"
#include "XCellRelocator.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"
#include "XCellResource.h"

#include <comdef.h>
#include <ScreenGrab11.h>

#define XCELL_DBG_POSTEFFECT 0							// Only debug
#define XCELL_TAA_NOSHARP 0
#define XCELL_TAA_NOBLURHISTORY 0
#define XCELL_USE_SMAA 1
#define XCELL_NO_USE_POSTEFFECTS_IN_PAUSEMENU 0			// Causes an artifact in the pause menu

#if XCELL_DBG_POSTEFFECT
#define XCELL_POSTEFFECT_MUL_SIZE 0.10f
#else
#define XCELL_POSTEFFECT_MUL_SIZE 1.0f
#endif // XCELL_DBG_POSTEFFECT

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

namespace XCell
{
	PostProcessor* GlobalPostProcessor = nullptr;

	///////////////////////////////////////////////////////////////////////////////

	RenderFrame::RenderFrame(const char* Name, const PostProcessor* Core) :
		Object(Name), _core(Core)
	{
		auto D3D11Device = Core->Device;
		auto D3D11DeviceContext = Core->DeviceContext;

		_colorBuffer	= make_unique<TextureShader>((this->Name + " frame color").c_str(), D3D11Device, D3D11DeviceContext);
		_depthBuffer	= make_unique<TextureShader>((this->Name + " frame depth").c_str(), D3D11Device, D3D11DeviceContext);
		_renderTarget	= make_unique<RenderTargetView>((this->Name + " frame color target").c_str(), D3D11Device, D3D11DeviceContext);
		_colorView		= make_unique<ResourceView>((this->Name + " frame color resource").c_str(), D3D11Device, D3D11DeviceContext);
		_depthView		= make_unique<ResourceView>((this->Name + " frame depth resource").c_str(), D3D11Device, D3D11DeviceContext);
	}

	bool RenderFrame::Initialize(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true)
	{
		if (_colorBuffer->Create(Color, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) && 
			_depthBuffer->Create(Depth))
		{
			CD3D11_RENDER_TARGET_VIEW_DESC rtvDescColor(D3D11_RTV_DIMENSION_TEXTURE2D, _colorBuffer->GetDesc()->Format);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDescColor(D3D11_SRV_DIMENSION_TEXTURE2D, _colorBuffer->GetDesc()->Format);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDescDepth(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

			return _renderTarget->Create(_colorBuffer->Get(), &rtvDescColor) &&
				_colorView->Create(_colorBuffer->Get(), &srvDescColor) &&
				_depthView->Create(_depthBuffer->Get(), &srvDescDepth);

			return true;
		}

		return false;
	}

	bool RenderFrame::CopyFrame(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true)
	{
		return _colorBuffer->CopyFrom(Color) && _depthBuffer->CopyFrom(Depth);
	}

	bool RenderFrame::CopyFrame(const RenderFrame* Frame) noexcept(true)
	{
		if (!Frame) return false;
		return _colorBuffer->CopyFrom(Frame->GetColor()) && _depthBuffer->CopyFrom(Frame->GetDepth());
	}

	void RenderFrame::DebugSaveFrameToFiles() const noexcept(true)
	{
		_colorBuffer->SaveTextureToFileAsDDS("debug_xcell_frame_color.dds");
		_depthBuffer->SaveTextureToFileAsDDS("debug_xcell_frame_depth.dds");
	}

	void RenderFrame::DebugInfo() const noexcept(true)
	{
		_colorBuffer->DebugInfo();
		_depthBuffer->DebugInfo();
		_colorView->DebugInfo();
		_depthView->DebugInfo();
	}

	///////////////////////////////////////////////////////////////////////////////

	HistoryFrames::HistoryFrames(const char* Name, const PostProcessor* Core) :
		Object(Name), _core(Core), _count(0)
	{
		_currFrame = make_unique<RenderFrame>("Current", Core);
		_prevFrame = make_unique<RenderFrame>("Previous", Core);
	}

	void HistoryFrames::StoreFrame(const ID3D11Resource* Color, const ID3D11Resource* Depth) noexcept(true)
	{
		IScopedCriticalSection Lock(&_CriticalSection);

		if (!_count)
		{
			if (!_currFrame->Initialize(Color, Depth) || !_prevFrame->Initialize(Color, Depth))
				_ERROR("HistoryFrames: Error init frames");
			else
			{
				_currFrame->DebugInfo();
				_currFrame->CopyFrame(Color, Depth);
			}
		}
		else
		{
			_prevFrame->CopyFrame(_currFrame.get());
			_currFrame->CopyFrame(Color, Depth);
		}

		_count++;
	}

	void HistoryFrames::ClearHistory() noexcept(true)
	{
		IScopedCriticalSection Lock(&_CriticalSection);

		_count = 0;
	}

	void HistoryFrames::DebugSaveCurrentFrameToFiles() const noexcept(true)
	{
		_currFrame->DebugSaveFrameToFiles();
	}

	PostProcessor::PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain,
		HWND RenderWindow) :
		_D3D11Device(D3D11Device), _D3D11DeviceContext(D3D11DeviceContext), _DXGISwapChain(DXGISwapChain), _RenderWindow(RenderWindow),
		_Width(0), _Height(0)
	{
		// Warning: GetClientRect() changed
		RECT rc;
		GetClientRect(_RenderWindow, &rc);
		auto SettingScale = CVarDisplayScale->GetFloat();
		_Width = rc.right / SettingScale;
		_Height = rc.bottom / SettingScale;

		_history = make_unique<HistoryFrames>("History", this);
		_taa = make_unique<TAA>("TAA", this);

		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));

		desc.FillMode = D3D11_FILL_SOLID;
		_D3D11Device->CreateRasterizerState(&desc, _D3D11RasterizerState.GetAddressOf());
	}

	void PostProcessor::Processing(ID3D11RenderTargetView* pRenderTarget, ID3D11Resource* pBackBuffer, 
		ID3D11Resource* pDepthStencilBuffer) noexcept(true)
	{
		// It should be skipped, since there is no depth buffer immediately.
		if (!pRenderTarget || !pBackBuffer || !pDepthStencilBuffer)
			return;

		// Setup viewport
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
		vp.Width = _Width;
		vp.Height = _Height;
		vp.MaxDepth = 1.0f;
		_D3D11DeviceContext->RSSetViewports(1, &vp);

		// Setup shader and vertex buffers
		_D3D11DeviceContext->GSSetShader(nullptr, nullptr, 0);
		_D3D11DeviceContext->HSSetShader(nullptr, nullptr, 0);
		_D3D11DeviceContext->DSSetShader(nullptr, nullptr, 0);
		_D3D11DeviceContext->CSSetShader(nullptr, nullptr, 0);

		// Setup blend state
		const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
		_D3D11DeviceContext->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
		_D3D11DeviceContext->OMSetDepthStencilState(nullptr, 0);
		_D3D11DeviceContext->RSSetState(_D3D11RasterizerState.Get());
		_D3D11DeviceContext->RSSetScissorRects(0, nullptr);

#if XCELL_NO_USE_POSTEFFECTS_IN_PAUSEMENU
		auto UIGame = *g_ui.GetPtr();
		if (!UIGame->IsMenuOpen("PauseMenu"))
		{
			_history->StoreFrame(pBackBuffer, pDepthStencilBuffer);
			_taa->Apply();
		}
		else if (_history->Count() > 0)
			_history->ClearHistory();
#else
		_history->StoreFrame(pBackBuffer, pDepthStencilBuffer);
		_taa->Apply();
#endif // XCELL_NO_USE_POSTEFFECTS_IN_PAUSEMENU
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
	}

	void PostProcessor::Shutdown()
	{}
}