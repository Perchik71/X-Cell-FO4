// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <XCellShader.h>

#include <wrl/client.h>

namespace XCell
{
	using namespace Microsoft::WRL;

	class ComputeShader : public Shader
	{
		ComPtr<ID3D11ComputeShader> _computeShader;
	public:
		ComputeShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~ComputeShader() = default;

		virtual void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) noexcept(true);
		virtual void Bind() noexcept(true);
		virtual void Unbind() noexcept(true);
		virtual bool Install() noexcept(true);
		virtual void Shutdown() noexcept(true);

		ComputeShader(const ComputeShader&) = delete;
		ComputeShader& operator=(const ComputeShader&) = delete;
	};
}