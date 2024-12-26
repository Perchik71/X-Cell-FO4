// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_fix_sampler.h>
#include <xc_version.h>
#include <xc_plugin.h>

#include <unordered_set>
#include <unordered_map>
#include <sdkddkver.h>
#include <wrl/client.h>

#include <f4se/PapyrusNativeFunctions.h>

using namespace Microsoft::WRL;

struct DirectXData
{
	char pad00[0x48];
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	char pad58[0x18];
	IDXGISwapChain* swap_chain;
};
static_assert(sizeof(DirectXData) == 0x78);

static DirectXData* d3d11_data = nullptr;
static uintptr_t sub_d3d11_after_init_ptr = 0;
static uintptr_t d3d11_sampler_subs_ptr[6] = { 0 };
static std::unordered_set<ID3D11SamplerState*> passThroughSamplers;
static std::unordered_map<ID3D11SamplerState*, ComPtr<ID3D11SamplerState>> mappedSamplers;
static xc::fix_sampler* __this = nullptr;
static float fLodBias = 0.0f;

namespace xc
{
	// Mostly from vrperfkit, thanks to fholger for showing how to do mip lod bias
	// https://github.com/fholger/vrperfkit/blob/037c09f3168ac045b5775e8d1a0c8ac982b5854f/src/d3d11/d3d11_post_processor.cpp#L76
	static void SetMipLodBias(ID3D11SamplerState** outSamplers, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		memcpy(outSamplers, ppSamplers, NumSamplers * sizeof(ID3D11SamplerState*));
		for (UINT i = 0; i < NumSamplers; ++i) 
		{
			auto orig = outSamplers[i];
			if (orig == nullptr || passThroughSamplers.find(orig) != passThroughSamplers.end())
				continue;
			
			if (mappedSamplers.find(orig) == mappedSamplers.end())
			{
				D3D11_SAMPLER_DESC sd;
				orig->GetDesc(&sd);

				if (sd.MipLODBias) 
				{
					// do not mess with samplers that already have a bias or are not doing anisotropic filtering.
					// should hopefully reduce the chance of causing rendering errors.
					passThroughSamplers.insert(orig);
					continue;
				}

				sd.MipLODBias = fLodBias;
				sd.MinLOD = 0;
				sd.MaxLOD = D3D11_FLOAT32_MAX;

				d3d11_data->device->CreateSamplerState(&sd, mappedSamplers[orig].GetAddressOf());
				passThroughSamplers.insert(mappedSamplers[orig].Get());
			}
			outSamplers[i] = mappedSamplers[orig].Get();
		}
	}

	void fix_sampler::hk_pssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[0], This, StartSlot, NumSamplers, samplers);
	}

	void fix_sampler::hk_vssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[1], This, StartSlot, NumSamplers, samplers);
	}

	void fix_sampler::hk_gssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[2], This, StartSlot, NumSamplers, samplers);
	}

	void fix_sampler::hk_hssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[3], This, StartSlot, NumSamplers, samplers);
	}

	void fix_sampler::hk_dssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[4], This, StartSlot, NumSamplers, samplers);
	}

	void fix_sampler::hk_cssetsamplers(ID3D11DeviceContext* This, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		SetMipLodBias(samplers, StartSlot, NumSamplers, ppSamplers);
		fastCall<void>(d3d11_sampler_subs_ptr[5], This, StartSlot, NumSamplers, samplers);
	}

	namespace PVM
	{
		static float GetMipLODBias(StaticFunctionTag* base)
		{
			return fLodBias;
		}

		static void SetMipLODBias(StaticFunctionTag* base, float value)
		{
			value = std::min(5.0f, std::max(-5.0f, value));

			_MESSAGE("MIP LOD Bias changed from %f to %f, recreating samplers", fLodBias, value);

			passThroughSamplers.clear();
			mappedSamplers.clear();

			fLodBias = value;
			g_plugin->write_setting_float("graphics", "miplodbias", fLodBias);
		}

		static void SetDefaultMipLODBias(StaticFunctionTag* base)
		{
			SetMipLODBias(base, 0.0f);
		}
	}

	void fix_sampler::register_functions(VirtualMachine* vm)
	{
		vm->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, float>("GetMipLODBias", "XCELL", PVM::GetMipLODBias, vm));
		vm->RegisterFunction(
			new NativeFunction1<StaticFunctionTag, void, float>("SetMipLODBias", "XCELL", PVM::SetMipLODBias, vm));
		vm->RegisterFunction(
			new NativeFunction0<StaticFunctionTag, void>("SetDefaultMipLODBias", "XCELL", PVM::SetDefaultMipLODBias, vm));
	}

	void fix_sampler::hk_sub_after_init()
	{
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// Получение структуры, очень похоже, что это класс NiRenderWindow
			d3d11_data = (DirectXData*)(g_plugin->get_base() + 0x61E0910);
			// Замена оригинальных функций установки сэмпла
			*(uintptr_t*)&d3d11_sampler_subs_ptr[0] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_pssetsamplers, 10);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[1] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_vssetsamplers, 26);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[2] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_gssetsamplers, 32);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[3] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_hssetsamplers, 61);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[4] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_dssetsamplers, 65);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[5] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context), 
				(uintptr_t)&hk_cssetsamplers, 70);
			// Продолжить выполнение перехваченой функции
			fastCall<void>(g_plugin->get_base() + 0x1D4FE40);
		} 
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// Получение структуры, очень похоже, что это класс NiRenderWindow
			d3d11_data = (DirectXData*)(g_plugin->get_base() + 0x3769610);
			// Замена оригинальных функций установки сэмпла
			*(uintptr_t*)&d3d11_sampler_subs_ptr[0] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_pssetsamplers, 10);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[1] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_vssetsamplers, 26);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[2] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_gssetsamplers, 32);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[3] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_hssetsamplers, 61);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[4] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_dssetsamplers, 65);
			*(uintptr_t*)&d3d11_sampler_subs_ptr[5] = __this->patch_vtable_func(*(uintptr_t*)(d3d11_data->context),
				(uintptr_t)&hk_cssetsamplers, 70);
			// Продолжить выполнение перехваченой функции
			fastCall<void>(g_plugin->get_base() + 0x1739EB0);
		}
	}

	const char* fix_sampler::get_name() const noexcept
	{
		return "mipbias";
	}

	bool fix_sampler::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool fix_sampler::run() const
	{
		__this = const_cast<fix_sampler*>(this);
		PVM::SetMipLODBias(nullptr, g_plugin->read_setting_float("graphics", "miplodbias", -1.3f));

#ifdef FO4NG2
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
			detour_call(g_plugin->get_base() + 0x16FB147, (uintptr_t)&hk_sub_after_init);
		else
		{
			_ERROR("The fix \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
#else
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
			detour_call(g_plugin->get_base() + 0x1D18A57, (uintptr_t)&hk_sub_after_init);
		else
		{
			_ERROR("The fix \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
#endif // FO4NG2
	}
}