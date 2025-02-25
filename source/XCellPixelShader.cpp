// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellPixelShader.h"

#include <comdef.h>

namespace XCell
{
	PixelShader::PixelShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Shader(Name, Device, DeviceContext)
	{}

	void PixelShader::Bind() noexcept(true)
	{
		DeviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
	}

	void PixelShader::Unbind() noexcept(true)
	{
		DeviceContext->PSSetShader(nullptr, nullptr, 0);
	}

	bool PixelShader::Install() noexcept(true)
	{
		if (IsEmpty())
			return false;

		// create the shader object
		auto hr = Device->CreatePixelShader(StreamBinary.Memory(), StreamBinary.Size, nullptr, _pixelShader.GetAddressOf());
		if (FAILED(hr))
		{
			_ERROR("CreatePixelShader: \"%s\" error has occurred: \"%s\"", Name.c_str(), _com_error(hr).ErrorMessage());
			return false;
		}

		_MESSAGE("PixelShader \"%s\" initialization was successful", Name.c_str());

		return true;
	}

	void PixelShader::Shutdown() noexcept(true)
	{
		_pixelShader.Reset();
	}
}