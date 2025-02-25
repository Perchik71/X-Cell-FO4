// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <XCellObject.h>

#include <wrl/client.h>
#include <d3d11.h>

namespace XCell
{
	using namespace Microsoft::WRL;

	enum : UInt32
	{
		XCELL_PIXEL_SHADER		= 1 << 0,
		XCELL_VERTEX_SHADER		= 1 << 1,
		XCELL_COMPUTE_SHADER	= 1 << 2,
	};

	class ObjectShader : public Object
	{
	protected:
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
	public:
		ObjectShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~ObjectShader() = default;

		virtual void InitPipeline(UInt32, UInt32) noexcept(true) = 0;
		virtual void ShutdownPipeline() noexcept(true) = 0;

		ObjectShader(const ObjectShader&) = delete;
		ObjectShader& operator=(const ObjectShader&) = delete;
	};
}