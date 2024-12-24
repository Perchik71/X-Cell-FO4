// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se/PapyrusVM.h>
#include <xc_patch.h>
#include <d3d11.h>

namespace xc
{
	class fix_sampler : public patch
	{
	public:
		fix_sampler() = default;
		fix_sampler(const fix_sampler&) = default;
		virtual ~fix_sampler() = default;

		fix_sampler& operator=(const fix_sampler&) = default;

		//static HRESULT hk_create_sampler_state(ID3D11Device* device, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);

		static void hk_pssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		static void hk_vssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		static void hk_gssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		static void hk_hssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		static void hk_dssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		static void hk_cssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);


		static void hk_sub_after_init();
		static void register_functions(VirtualMachine* vm);

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	};
}