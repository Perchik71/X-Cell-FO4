// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellComputeShader.h"

#include <comdef.h>

namespace XCell
{
	ComputeShader::ComputeShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Shader(Name, Device, DeviceContext)
	{}

	void ComputeShader::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) noexcept(true)
	{
		DeviceContext->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}

	void ComputeShader::Bind() noexcept(true)
	{
		DeviceContext->CSSetShader(_computeShader.Get(), nullptr, 0);
	}

	void ComputeShader::Unbind() noexcept(true)
	{
		DeviceContext->CSSetShader(nullptr, nullptr, 0);
	}

	bool ComputeShader::Install() noexcept(true)
	{
		if (IsEmpty())
			return false;

		// create the shader object
		auto hr = Device->CreateComputeShader(StreamBinary.Memory(), StreamBinary.Size, nullptr, _computeShader.GetAddressOf());
		if (FAILED(hr))
		{
			_ERROR("CreateComputeShader: \"%s\" error has occurred: \"%s\"", Name.c_str(), _com_error(hr).ErrorMessage());
			return false;
		}

		_MESSAGE("ComputeShader \"%s\" initialization was successful", Name.c_str());

		return true;
	}

	void ComputeShader::Shutdown() noexcept(true)
	{
		_computeShader.Reset();
	}
}