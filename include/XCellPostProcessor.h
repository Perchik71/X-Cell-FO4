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

	class PostProcessor
	{
		ID3D11Device* _D3D11Device;
		ID3D11DeviceContext* _D3D11DeviceContext;
		IDXGISwapChain* _DXGISwapChain;
		UInt64 _OldFunctions[6];

		unordered_set<ID3D11SamplerState*> PassThroughSamplers;
		unordered_map<ID3D11SamplerState*, ComPtr<ID3D11SamplerState>> MappedSamplers;
	public:
		PostProcessor(ID3D11Device* D3D11Device, ID3D11DeviceContext* D3D11DeviceContext, IDXGISwapChain* DXGISwapChain);

		virtual void PreXSSetSamplers(ID3D11SamplerState** OutSamplers, UINT StartSlot, UINT NumSamplers, 
			ID3D11SamplerState* const* Samplers);
		virtual void NativeCallXSSetSamplers(UInt8 Index, ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers,
			ID3D11SamplerState* const* Samplers);

		virtual void Install();
		virtual void Shutdown();
		virtual void Reset();
	} extern *GlobalPostProcessor;
}