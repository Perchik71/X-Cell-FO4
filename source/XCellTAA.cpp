// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellCommonType.h"
#include "XCellPostProcessor.h"
#include "XCellTAA.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"
#include "XCellResource.h"

#include <comdef.h>
#include <ScreenGrab11.h>

#define XCELL_DBG_TAA_POSTEFFECT 0							// Only debug

#if XCELL_DBG_TAA_POSTEFFECT
#define XCELL_TAA_POSTEFFECT_MUL_SIZE 0.10f
#else
#define XCELL_TAA_POSTEFFECT_MUL_SIZE 1.0f
#endif // XCELL_DBG_TAA_POSTEFFECT

namespace XCell
{
	///////////////////////////////////////////////////////////////////////////////

	TAA::TAA(const char* Name, PostProcessor* PostProcessor) : Object(Name),
		_postProcessor(PostProcessor)
	{
		auto D3D11Device = _postProcessor->Device;
		auto D3D11DeviceContext = _postProcessor->DeviceContext;

		_outputMainRV = make_unique<ResourceView>((this->Name + ": main rv").c_str(), D3D11Device, D3D11DeviceContext);
		_outputSharpenerRV = make_unique<ResourceView>((this->Name + ": sharpener rv").c_str(), D3D11Device, D3D11DeviceContext);
		_mainCS = make_unique<ComputeShader>((this->Name + ": main shader").c_str(), D3D11Device, D3D11DeviceContext);
		_sharpenerCS = make_unique<ComputeShader>((this->Name + ": sharpener shader").c_str(), D3D11Device, D3D11DeviceContext);
		_pointSampler = make_unique<SamplerState>((this->Name + ": point sampler").c_str(), D3D11Device, D3D11DeviceContext);
		_linearSampler = make_unique<SamplerState>((this->Name + ": linear sampler").c_str(), D3D11Device, D3D11DeviceContext);

		float BorderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		CD3D11_SAMPLER_DESC pointSamplerDesc(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0, 1, D3D11_COMPARISON_NEVER, BorderColor, -FLT_MAX, FLT_MAX);
		_pointSampler->Create(&pointSamplerDesc);
		CD3D11_SAMPLER_DESC linearSamplerDesc(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0, 1, D3D11_COMPARISON_NEVER, BorderColor, -FLT_MAX, FLT_MAX);
		_linearSampler->Create(&linearSamplerDesc);

		if (!_mainCS->LoadFromResource(IDR_CS_SHADER_TAA_MAIN))
			_ERROR((this->Name + ": shader %u loading error").c_str(), IDR_CS_SHADER_TAA_MAIN);

		if (!_sharpenerCS->LoadFromResource(IDR_CS_SHADER_TAA_SHARPENER))
			_ERROR((this->Name + ": shader %u loading error").c_str(), IDR_CS_SHADER_TAA_SHARPENER);

		_mainCS->Install();
		_sharpenerCS->Install();

		_outputMain = make_unique<UnorderedAccessView>((this->Name + ": main uv").c_str(), D3D11Device, D3D11DeviceContext);
		_outputSharpener = make_unique<UnorderedAccessView>((this->Name + ": sharpener uv").c_str(), D3D11Device, D3D11DeviceContext);

		RECT rc;
		GetClientRect(_postProcessor->WindowHandle, &rc);

		ThreadX = (((UINT)rc.right) + 15) >> 4;
		ThreadY = (((UINT)rc.bottom) + 15) >> 4;

		CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, (UINT)rc.right, (UINT)rc.bottom, 1, 0,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		_outputMainTex = make_unique<TextureShader>((this->Name + ": output main texture").c_str(), D3D11Device, D3D11DeviceContext);
		_outputSharpenerTex = make_unique<TextureShader>((this->Name + ": output sharpener texture").c_str(), D3D11Device, D3D11DeviceContext);
		
		if (_outputMainTex->Create(&textureDesc) && _outputSharpenerTex->Create(&textureDesc))
			_MESSAGE((this->Name + ": create textures [%ux%u] was successful").c_str(), (UINT)rc.right, (UINT)rc.bottom);

		CD3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc(D3D_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
		_outputMainRV->Create(_outputMainTex->Get(), &resourceDesc);
		_outputSharpenerRV->Create(_outputSharpenerTex->Get(), &resourceDesc);

		CD3D11_UNORDERED_ACCESS_VIEW_DESC uvDesc(D3D11_UAV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
		_outputMain->Create(_outputMainTex->Get(), &uvDesc);
		_outputSharpener->Create(_outputSharpenerTex->Get(), &uvDesc);

		_mainVS = make_unique<VertexShader>((this->Name + ": vertex shader").c_str(), D3D11Device, D3D11DeviceContext);
		if (_mainVS)
		{
			if (!_mainVS->LoadFromResource(IDR_VS_SHADER_COMMON))
				_mainVS.release();
			else
			{
				_mainVS->Install();
#if XCELL_DBG_TAA_POSTEFFECT
				_mainVS->InitData(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, {
					{ -1.0f, -0.5f * XCELL_TAA_POSTEFFECT_MUL_SIZE }, { 0.0f, 0.0f },										// point at top-left
					{ -0.5f * XCELL_TAA_POSTEFFECT_MUL_SIZE, -0.5f * XCELL_TAA_POSTEFFECT_MUL_SIZE }, { 1.0f, 0.0f },		// point at top-right
					{ -1.0f, -1.0f }, { 0.0f, 1.0f },																		// point at bottom-left
					{ -0.5f * XCELL_TAA_POSTEFFECT_MUL_SIZE, -1.0f }, { 1.0f, 1.0f },										// point at bottom-right
					});
#else
				_mainVS->InitData(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, {
					{ -1.0f, 1.0f }, { 0.0f, 0.0f },	// point at top-left
					{ 1.0f, 1.0f }, { 1.0f, 0.0f },		// point at top-right
					{ -1.0f, -1.0f }, { 0.0f, 1.0f },	// point at bottom-left
					{ 1.0f, -1.0f }, { 1.0f, 1.0f },	// point at bottom-right
					});
#endif
			}
		}

		_mainPS = make_unique<PixelShader>((this->Name + ": pixel shader").c_str(), D3D11Device, D3D11DeviceContext);
		if (!_mainPS->LoadFromResource(IDR_PS_SHADER_COMMON))
			_mainPS.release();
		else
			_mainPS->Install();
	}

	void TAA::Apply() const noexcept(true)
	{
		auto D3D11Device = _postProcessor->Device;
		auto D3D11DeviceContext = _postProcessor->DeviceContext;
		auto History = _postProcessor->History;

		if ((History->Count() < 2) || !CVarTAA->GetBool())
			return;
		
		// TAA main blur

		History->CurrentFrame()->InitColorPipeline(XCELL_COMPUTE_SHADER, 0);
		History->CurrentFrame()->InitDepthPipeline(XCELL_COMPUTE_SHADER, 1);
		History->PreviousFrame()->InitColorPipeline(XCELL_COMPUTE_SHADER, 2);
		History->PreviousFrame()->InitDepthPipeline(XCELL_COMPUTE_SHADER, 3);
			
		_pointSampler->InitPipeline(XCELL_COMPUTE_SHADER, 0);
		_outputMain->InitPipeline(XCELL_COMPUTE_SHADER, 0);

		_mainCS->Bind();
		_mainCS->Dispatch(ThreadX, ThreadY, 1);
		_outputMain->ShutdownPipeline();

		// TAA sharp
#if 0

		_outputSharpener->InitPipeline(XCELL_COMPUTE_SHADER, 0);
		_outputMainRV->InitPipeline(XCELL_COMPUTE_SHADER, 0);

		_sharpenerCS->Bind();
		_sharpenerCS->Dispatch(ThreadX, ThreadY, 1);
		
		_sharpenerCS->Unbind();
		_outputSharpener->ShutdownPipeline();
		_outputMainRV->ShutdownPipeline();

		// _outputSharpenerRV doesn't want to work
		_outputMainTex->CopyFrom(_outputSharpenerTex->Get());
		_outputMainRV->InitPipeline(XCELL_PIXEL_SHADER, 0);
#else
		_outputSharpenerTex->CopyFrom(_outputMainTex->Get());
		_outputSharpenerRV->InitPipeline(XCELL_PIXEL_SHADER, 0);
#endif

		// Output
		//_outputMainTex->SaveTextureToFileAsDDS("test.dds");
		
		_linearSampler->InitPipeline(XCELL_PIXEL_SHADER, 0);

		_mainVS->Bind();
		_mainPS->Bind();

		D3D11DeviceContext->Draw(4, 0);
	}
}