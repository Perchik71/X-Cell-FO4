// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include "XCellObjectShader.h"

namespace XCell
{
	class BufferShader : public ObjectShader
	{
		UINT IPType;
		ComPtr<ID3D11Buffer> _Buffer;
	public:
		BufferShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~BufferShader() = default;

		virtual bool Create(const void* Buffer, UINT Size, UINT BindFlags, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT) noexcept(true);
		virtual void InitPipeline(UInt32 Type, UINT BufferID) noexcept(true);
		virtual void ShutdownPipeline(UINT BufferID) noexcept(true);

		[[nodiscard]] virtual ID3D11Buffer* Get() const noexcept(true) { return _Buffer.Get(); }
		[[nodiscard]] virtual ID3D11Buffer** GetAddressOf() noexcept(true) { return _Buffer.GetAddressOf(); }
		[[nodiscard]] virtual ID3D11Buffer** ReleaseAndGetAddressOf() noexcept(true) { return _Buffer.ReleaseAndGetAddressOf(); }

		BufferShader(const BufferShader&) = delete;
		BufferShader& operator=(const BufferShader&) = delete;
	};
}