// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <wrl/client.h>
#include <d3d11.h>

// F4SE
#include <f4se/PapyrusVM.h>

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

#include <unordered_map>
#include <unordered_set>

namespace XCell
{
	using namespace Microsoft::WRL;

	class ModuleControlSamples : public Module
	{
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
		UInt64 _OldFunctions[6];
		unordered_set<ID3D11SamplerState*> PassThroughSamplers;
		unordered_map<ID3D11SamplerState*, ComPtr<ID3D11SamplerState>> MappedSamplers;
	public:
		static constexpr auto SourceName = "Module Control Samples";

		ModuleControlSamples(void* Context);
		virtual ~ModuleControlSamples() = default;

		ModuleControlSamples(const ModuleControlSamples&) = delete;
		ModuleControlSamples& operator=(const ModuleControlSamples&) = delete;

		void PreXSSetSamplers(ID3D11SamplerState** OutSamplers, UINT StartSlot, UINT NumSamplers,
			ID3D11SamplerState* const* Samplers);
		void NativeCallXSSetSamplers(UInt8 Index, UINT StartSlot, UINT NumSamplers,
			ID3D11SamplerState* const* Samplers);
		void Reset();

		virtual HRESULT DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
			IDXGISwapChain* SwapChain);
		virtual HRESULT VMListener(VirtualMachine* VM);
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}