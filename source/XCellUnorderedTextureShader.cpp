// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellUnorderedTextureShader.h"

#include <ScreenGrab11.h>
#include <comdef.h>

#include <memory>

namespace XCell
{
	UnorderedTextureShader::UnorderedTextureShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), Counts(0), IPType(0)
	{}

	bool UnorderedTextureShader::CreateFromTextureDescription(const D3D11_TEXTURE2D_DESC& TextureDesc) noexcept(true)
	{
		memcpy(&_desc, &TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		if (SUCCEEDED(Device->CreateTexture2D(&TextureDesc, nullptr, TextureReleaseAndGetAddressOf())))
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
			ZeroMemory(&descUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

			descUAV.Format = TextureDesc.Format;
			descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

			auto hr = Device->CreateUnorderedAccessView(GetTexture(), &descUAV, _View.GetAddressOf());
			if (FAILED(hr))
			{
				_ERROR("UnorderedTextureShader: \"%s\" CreateUnorderedAccessView returned failed \"%s\"", Name.c_str(),
					_com_error(hr).ErrorMessage());
				return false;
			}

			return true;
		}

		return false;
	}

	bool UnorderedTextureShader::CreateFromTexture(const ID3D11Texture2D* Texture) noexcept(true)
	{
		if (!Texture)
			return false;

		D3D11_TEXTURE2D_DESC TexDesc;
		auto Tex = const_cast<ID3D11Texture2D*>(Texture);
		Tex->GetDesc(&TexDesc);

		if (!CreateFromTextureDescription(TexDesc))
			return false;

		DeviceContext->CopyResource(GetTexture(), Tex);

		return true;
	}

	bool UnorderedTextureShader::CreateFromResource(const ID3D11Resource* Resource) noexcept(true)
	{
		if (!Resource)
			return false;

		ComPtr<ID3D11Texture2D> Tex;
		auto Res = const_cast<ID3D11Resource*>(Resource);
		auto hr = Res->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(Tex.GetAddressOf()));
		if (FAILED(hr))
			return false;

		return CreateFromTexture(Tex.Get());
	}

	bool UnorderedTextureShader::SaveTextureToFileAsDDS(const char* FileName) noexcept(true)
	{
		if (!FileName || !_texture || !DeviceContext)
			return false;

		int len = MultiByteToWideChar(CP_ACP, 0, FileName, -1, nullptr, 0);
		if (len <= 0)
			return false;

		auto str = make_unique<wchar_t[]>((size_t)len + 1);
		if (!str)
			return false;

		MultiByteToWideChar(CP_ACP, 0, FileName, len, str.get(), len);
		return SUCCEEDED(DirectX::SaveDDSTextureToFile(DeviceContext, GetTexture(), str.get()));
	}

	bool UnorderedTextureShader::CopyFrom(const ID3D11Resource* Resource) noexcept(true)
	{
		if (!Resource || !DeviceContext || !_texture)
			return false;

		DeviceContext->CopyResource(_texture.Get(), const_cast<ID3D11Resource*>(Resource));
		return true;
	}

	bool UnorderedTextureShader::CopyFrom(const ID3D11Texture2D* Texture) noexcept(true)
	{
		if (!Texture || !DeviceContext || !_texture)
			return false;

		DeviceContext->CopyResource(_texture.Get(), const_cast<ID3D11Texture2D*>(Texture));
		return true;
	}

	bool UnorderedTextureShader::CopyFrom(const UnorderedTextureShader* Texture) noexcept(true)
	{
		if (!Texture) return false;
		return CopyFrom(Texture->GetTexture());
	}

	void UnorderedTextureShader::InitPipeline(UInt32 Type, UINT TexID) noexcept(true)
	{
		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetUnorderedAccessViews(TexID, 1, _View.GetAddressOf(), &Counts);

		IPType = Type;
	}

	void UnorderedTextureShader::ShutdownPipeline(UINT TexID) noexcept(true)
	{
		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetUnorderedAccessViews(TexID, 0, nullptr, nullptr);

		IPType = 0;
	}
}