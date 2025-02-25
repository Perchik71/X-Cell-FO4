// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellBufferShader.h"

#include <comdef.h>

namespace XCell
{
	BufferShader::BufferShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), IPType(0)
	{}

	bool BufferShader::Create(const void* Buffer, UINT Size, UINT BindFlags, D3D11_USAGE Usage) noexcept(true)
	{
		// Fill in a buffer description
		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));
		cbDesc.ByteWidth = Size;
		cbDesc.Usage = Usage;
		cbDesc.BindFlags = BindFlags;

		// Fill in the subresource data
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
		InitData.pSysMem = Buffer;

		// Create the buffer
		auto hr = Device->CreateBuffer(&cbDesc, &InitData, _Buffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			_ERROR("BufferShader: \"%s\" CreateBuffer returned failed \"%s\"", Name.c_str(), 
				_com_error(hr).ErrorMessage());
			return false;
		}

		return true;
	}

	void BufferShader::InitPipeline(UInt32 Type, UINT BufferID) noexcept(true)
	{
		if ((Type & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
			DeviceContext->PSSetConstantBuffers(BufferID, 1, _Buffer.GetAddressOf());

		if ((Type & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
			DeviceContext->VSSetConstantBuffers(BufferID, 1, _Buffer.GetAddressOf());

		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetConstantBuffers(BufferID, 1, _Buffer.GetAddressOf());

		IPType = Type;
	}

	void BufferShader::ShutdownPipeline(UINT BufferID) noexcept(true)
	{
		if ((IPType & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
			DeviceContext->PSSetConstantBuffers(BufferID, 0, nullptr);

		if ((IPType & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
			DeviceContext->VSSetConstantBuffers(BufferID, 0, nullptr);

		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetConstantBuffers(BufferID, 0, nullptr);

		IPType = 0;
	}
}