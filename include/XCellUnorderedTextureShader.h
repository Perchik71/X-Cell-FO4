// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include "XCellObjectShader.h"

namespace XCell
{
	class UnorderedTextureShader : public ObjectShader
	{
		UINT Counts, IPType;
		D3D11_TEXTURE2D_DESC _desc;
		ComPtr<ID3D11Texture2D> _texture;
		ComPtr<ID3D11UnorderedAccessView> _View;
	public:
		UnorderedTextureShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~UnorderedTextureShader() = default;

		virtual bool CreateFromTextureDescription(const D3D11_TEXTURE2D_DESC& TextureDesc) noexcept(true);
		virtual bool CreateFromTexture(const ID3D11Texture2D* Texture) noexcept(true);
		virtual bool CreateFromResource(const ID3D11Resource* Resource) noexcept(true);

		virtual bool SaveTextureToFileAsDDS(const char* FileName) noexcept(true);

		virtual bool CopyFrom(const ID3D11Resource* Resource) noexcept(true);
		virtual bool CopyFrom(const ID3D11Texture2D* Texture) noexcept(true);
		virtual bool CopyFrom(const UnorderedTextureShader* Texture) noexcept(true);

		virtual void InitPipeline(UInt32 Type, UINT TexID) noexcept(true);
		virtual void ShutdownPipeline(UINT TexID) noexcept(true);

		[[nodiscard]] virtual inline ID3D11Texture2D* GetTexture() const noexcept(true) { return _texture.Get(); }
		[[nodiscard]] virtual inline ID3D11Texture2D** GetTextureAddressOf() noexcept(true) { return _texture.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11Texture2D** TextureReleaseAndGetAddressOf() noexcept(true) { return _texture.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView* GetView() const noexcept(true) { return _View.Get(); }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView** GetViewAddressOf() noexcept(true) { return _View.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView** ViewReleaseAndGetAddressOf() noexcept(true) { return _View.ReleaseAndGetAddressOf(); }

		UnorderedTextureShader(const UnorderedTextureShader&) = delete;
		UnorderedTextureShader& operator=(const UnorderedTextureShader&) = delete;
	};
}