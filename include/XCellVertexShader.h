// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <XCellShader.h>
#include <XCellClassesShader.h>

#include <memory>

namespace XCell
{
	class VertexShader : public Shader
	{
		ComPtr<ID3D11VertexShader> _vertexShader;
		ComPtr<ID3D11InputLayout> _inputLayout;
		unique_ptr<BufferShader> _inputBuffer;
		unique_ptr<BufferShader> _indexesBuffer;
		UINT _sizeBuffer;
		UINT _strideBuffer;
		UINT _offsetBuffer;
		D3D_PRIMITIVE_TOPOLOGY _topology;
	public:
		VertexShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~VertexShader() = default;

		virtual void Bind() noexcept(true);
		virtual void Unbind() noexcept(true);
		virtual bool Install() noexcept(true);
		virtual void Shutdown() noexcept(true);

		virtual bool InitData(D3D_PRIMITIVE_TOPOLOGY Topology, const initializer_list<Vector2>& InputData) noexcept(true);
		virtual bool InitDataWithIndexes(D3D_PRIMITIVE_TOPOLOGY Topology, const initializer_list<Vector2>& InputData, 
			const initializer_list<UINT>& Indexes) noexcept(true);

		VertexShader(const VertexShader&) = delete;
		VertexShader& operator=(const VertexShader&) = delete;
	};
}