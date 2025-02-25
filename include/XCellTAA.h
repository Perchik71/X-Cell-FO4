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
	class TAA : public Object
	{
		const PostProcessor* _postProcessor;
		unique_ptr<ComputeShader> _mainCS;
		unique_ptr<ComputeShader> _sharpenerCS;
		unique_ptr<SamplerState> _pointSampler;
		unique_ptr<SamplerState> _linearSampler;
		unique_ptr<UnorderedAccessView> _outputMain;
		unique_ptr<UnorderedAccessView> _outputSharpener;
		unique_ptr<TextureShader> _outputMainTex;
		unique_ptr<TextureShader> _outputSharpenerTex;
		unique_ptr<VertexShader> _mainVS;
		unique_ptr<PixelShader> _mainPS;
		unique_ptr<ResourceView> _outputMainRV;
		//unique_ptr<ResourceView> _outputSharpenerRV;
		UINT ThreadX, ThreadY;
	public:
		TAA(const char* Name, PostProcessor* PostProcessor);
		virtual ~TAA() = default;

		virtual void Apply() const noexcept(true);

		TAA(const TAA&) = delete;
		TAA& operator=(const TAA&) = delete;
	};
}