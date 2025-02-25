// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#define NOUSE_DX12_2 1

#include <d3d11.h>
#include <d3d11_2.h>
#include <dxgi.h>
#include <dxgi1_2.h>

namespace XCell
{
	struct DirectXData
	{
		char pad00[0x48];
#if NOUSE_DX12_2
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
#else
		ID3D11Device2* device = nullptr;
		ID3D11DeviceContext2* context = nullptr;
#endif
		HWND window;
		UINT posx;
		UINT posy;
		UINT width;
		UINT height;
#if NOUSE_DX12_2
		IDXGISwapChain* swap_chain = nullptr;
#else
		IDXGISwapChain1* swap_chain = nullptr;
#endif
		char pad78[0x1D68];
		ID3D11Resource* depth_stencil_resource = nullptr;					// format DXGI_FORMAT_D24_UNORM_S8_UINT
		ID3D11DepthStencilView* depth_stencil_view = nullptr;				// desc with flag 0 (default)
		char pad1DF0[0x18];
		ID3D11DepthStencilView* depth_stencil_view_r0_depth = nullptr;		// desc with flag D3D11_DSV_READ_ONLY_DEPTH
		char pad1E10[0x18];
		ID3D11DepthStencilView* depth_stencil_view_ro_stencil = nullptr;	// desc with flag D3D11_DSV_READ_ONLY_STENCIL
		char pad1E30[0x18];
		ID3D11DepthStencilView* depth_stencil_view_dup = nullptr;			// desc with flag 3 (WHAT???)
		char pad1E50[0x18];
	};  
	static_assert(sizeof(DirectXData) == 0x1E68);

	extern DirectXData* gDirectXData;
}
