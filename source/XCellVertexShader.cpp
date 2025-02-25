// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellVertexShader.h"

#include <comdef.h>
#include <memory>

namespace XCell
{
	VertexShader::VertexShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Shader(Name, Device, DeviceContext)
	{
		_inputBuffer = make_unique<BufferShader>(Name, Device, DeviceContext);
		_indexesBuffer = make_unique<BufferShader>(Name, Device, DeviceContext);
	}

	void VertexShader::Bind() noexcept(true)
	{
		DeviceContext->IASetPrimitiveTopology(_topology);
		DeviceContext->IASetInputLayout(_inputLayout.Get());
		DeviceContext->IASetVertexBuffers(0, 1, _inputBuffer->GetAddressOf(), &_strideBuffer, &_offsetBuffer);
		DeviceContext->IASetIndexBuffer(_indexesBuffer->Get(), DXGI_FORMAT_R32_UINT, 0);
		DeviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	}

	void VertexShader::Unbind() noexcept(true)
	{
		DeviceContext->IASetPrimitiveTopology(_topology);
		DeviceContext->IASetInputLayout(nullptr);
		DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
		DeviceContext->VSSetShader(nullptr, nullptr, 0);
	}

	bool VertexShader::Install() noexcept(true)
	{
		if (IsEmpty())
			return false;

		// create the shader object
		auto hr = Device->CreateVertexShader(StreamBinary.Memory(), StreamBinary.Size, nullptr, _vertexShader.GetAddressOf());
		if (FAILED(hr))
		{
			_ERROR("CreateVertexShader: \"%s\" error has occurred: \"%s\"", Name.c_str(), _com_error(hr).ErrorMessage());
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = Device->CreateInputLayout(inputElementDesc, _ARRAYSIZE(inputElementDesc), StreamBinary.Memory(), StreamBinary.Size,
			_inputLayout.GetAddressOf());
		if (FAILED(hr))
		{
			_ERROR("CreateInputLayout: \"%s\" error has occurred: \"%s\"", Name.c_str(), _com_error(hr).ErrorMessage());
			return false;
		}

		_MESSAGE("VertexShader \"%s\" initialization was successful", Name.c_str());

		return true;
	}

	void VertexShader::Shutdown() noexcept(true)
	{
		_vertexShader.Reset();
	}

	bool VertexShader::InitData(D3D_PRIMITIVE_TOPOLOGY Topology, const initializer_list<Vector2>& InputData) noexcept(true)
	{
		if (!InputData.size())
			return false;

		_topology = Topology;
		_sizeBuffer = InputData.size() >> 1;
		_strideBuffer = sizeof(Vector2) + sizeof(Vector2);
		_offsetBuffer = 0;

		if (!_inputBuffer->Create(reinterpret_cast<const void*>(InputData.begin()),
			_strideBuffer * _sizeBuffer, D3D11_BIND_VERTEX_BUFFER))
			return false;

		_MESSAGE("VertexShader \"%s\" initialization data was successful", Name.c_str());

		return true;
	}

	bool VertexShader::InitDataWithIndexes(D3D_PRIMITIVE_TOPOLOGY Topology, const initializer_list<Vector2>& InputData,
		const initializer_list<UINT>& Indexes) noexcept(true)
	{
		if (!Indexes.size() || !InitData(Topology, InputData))
			return false;

		if (!_indexesBuffer->Create(reinterpret_cast<const void*>(InputData.begin()),
			sizeof(UINT) * InputData.size(), D3D11_BIND_VERTEX_BUFFER))
			return false;

		_MESSAGE("VertexShader \"%s\" initialization indexes was successful", Name.c_str());

		return true;
	}
}