// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellClassesShader.h"

#include <ScreenGrab11.h>
#include <comdef.h>

#include <memory>

#define XCELL_TEXTURE_COPY_WITHOUT_CONVERSION 1

namespace XCell
{
	static bool CheckHrResult(HRESULT hr, const char* ClassName)
	{
		bool Result = SUCCEEDED(hr);
		if (!Result)
			_ERROR("An error has occurred in the \"%s\" class. Message: \"%s\"", ClassName, _com_error(hr).ErrorMessage());
		return Result;
	}

	/// =====================================

	ObjectDXCustom::ObjectDXCustom(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Object(Name), Device(Device), DeviceContext(DeviceContext)
	{}

	/// =====================================

	ObjectShader::ObjectShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectDXCustom(Name, Device, DeviceContext)
	{}

	/// =====================================

	TextureShader::TextureShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectDXCustom(Name, Device, DeviceContext)
	{}

	bool TextureShader::SaveTextureToFileAsDDS(const char* FileName) const noexcept(true)
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
		return CheckHrResult(DirectX::SaveDDSTextureToFile(DeviceContext, Get(), str.get()), Name.c_str());
	}

	void TextureShader::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Texture \"%s\" info: Size(%ux%u) Format(%X) Usage(%X) BindFlags(%X)",
			Name.c_str(), _desc.Width, _desc.Height, _desc.Format, _desc.Usage, _desc.BindFlags);
	}

	bool TextureShader::Create(const D3D11_TEXTURE2D_DESC* Desc) noexcept(true)
	{
		if (!Desc)
		{
			_ERROR("TextureShader: \"%s\" description nullptr", Name.c_str());
			return false;
		}

		memcpy(&_desc, Desc, sizeof(D3D11_TEXTURE2D_DESC));
		return CheckHrResult(Device->CreateTexture2D(&_desc, nullptr, ReleaseAndGetAddressOf()), Name.c_str());
	}

	bool TextureShader::Create(const ID3D11Texture2D* Texture, UInt32 BindFlags) noexcept(true)
	{
		if (!Texture)
		{
			_ERROR("TextureShader: \"%s\" texture nullptr", Name.c_str());
			return false;
		}

		auto Tex = const_cast<ID3D11Texture2D*>(Texture);
		Tex->GetDesc(&_desc);
		_desc.BindFlags = BindFlags;

		if (Create(&_desc))
		{
			DeviceContext->CopyResource(Get(), Tex);
			return true;
		}

		return false;
	}

	bool TextureShader::Create(const ID3D11Resource* Resource, UInt32 BindFlags) noexcept(true)
	{
		if (!Resource)
		{
			_ERROR("TextureShader: \"%s\" resource nullptr", Name.c_str());
			return false;
		}

#if XCELL_TEXTURE_COPY_WITHOUT_CONVERSION
		auto Res = const_cast<ID3D11Resource*>(Resource);
#else
		D3D11_RESOURCE_DIMENSION dimType;
		auto Res = const_cast<ID3D11Resource*>(Resource);
		Res->GetType(&dimType);
		if (dimType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
			return false;
#endif // XCELL_TEXTURE_COPY_WITHOUT_CONVERSION

		ComPtr<ID3D11Texture2D> Tex;	
		auto hr = Res->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(Tex.GetAddressOf()));
		if (CheckHrResult(hr, Name.c_str()))
			return Create(Tex.Get(), BindFlags);
		return false;
	}

	bool TextureShader::CopyFrom(const TextureShader* Texture) noexcept(true)
	{
		if (!Texture) return false;
		return CopyFrom(Texture->Get());
	}

	bool TextureShader::CopyFrom(const ID3D11Texture2D* Texture) noexcept(true)
	{
		if (!Texture) return false;
		DeviceContext->CopyResource(Get(), const_cast<ID3D11Texture2D*>(Texture));
		return true;
	}

	bool TextureShader::CopyFrom(const ID3D11Resource* Resource) noexcept(true)
	{
		if (!Resource)
			return false;

#if XCELL_TEXTURE_COPY_WITHOUT_CONVERSION
		DeviceContext->CopyResource(Get(), const_cast<ID3D11Resource*>(Resource));
#else
		D3D11_RESOURCE_DIMENSION dimType;
		auto Res = const_cast<ID3D11Resource*>(Resource);
		Res->GetType(&dimType);
		if (dimType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
			return false;

		ComPtr<ID3D11Texture2D> Tex;
		auto hr = Res->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(Tex.GetAddressOf()));
		if (!CheckHrResult(hr, Name.c_str()))
			return false;

		DeviceContext->CopyResource(Get(), Tex.Get());
#endif // XCELL_TEXTURE_COPY_WITHOUT_CONVERSION	

		return true;
	}

	/// =====================================

	BufferShader::BufferShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), IPType(0)
	{}

	bool BufferShader::Create(const void* Buffer, UInt32 Size, UInt32 BindFlags, D3D11_USAGE Usage) noexcept(true)
	{
		if (!Buffer || !Size)
			return false;

		CD3D11_BUFFER_DESC cbDesc(Size, BindFlags, Usage);
		memcpy(&_desc, &cbDesc, sizeof(D3D11_BUFFER_DESC));

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
		InitData.pSysMem = Buffer;

		return CheckHrResult(Device->CreateBuffer(&_desc, &InitData, _Buffer.ReleaseAndGetAddressOf()), Name.c_str());
	}

	void BufferShader::InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true)
	{
		if ((Type & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
		{
			DeviceContext->PSSetConstantBuffers(BindID, 1, GetAddressOf());
			BindId[0] = BindID;
		}

		if ((Type & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
		{
			DeviceContext->VSSetConstantBuffers(BindID, 1, GetAddressOf());
			BindId[1] = BindID;
		}

		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
		{
			DeviceContext->CSSetConstantBuffers(BindID, 1, GetAddressOf());
			BindId[2] = BindID;
		}

		IPType = Type;
	}

	void BufferShader::ShutdownPipeline() noexcept(true)
	{
		if ((IPType & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
			DeviceContext->PSSetConstantBuffers(BindId[0], 0, nullptr);

		if ((IPType & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
			DeviceContext->VSSetConstantBuffers(BindId[1], 0, nullptr);

		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetConstantBuffers(BindId[2], 0, nullptr);

		IPType = 0;
	}

	/// =====================================

	ResourceView::ResourceView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), IPType(0)
	{}

	void ResourceView::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Resource \"%s\" info: Format(%X) Buffer([%u:%u] Off:%u Width:%u) ViewDimension(%X)", Name.c_str(),
			_desc.Format, _desc.Buffer.FirstElement, _desc.Buffer.NumElements, _desc.Buffer.ElementOffset, _desc.Buffer.ElementWidth,
			_desc.ViewDimension);
	}

	bool ResourceView::Create(const ID3D11Resource* Resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* Desc) noexcept(true)
	{
		if (!Resource || !Desc)
			return false;

		memcpy(&_desc, Desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		return CheckHrResult(Device->CreateShaderResourceView(const_cast<ID3D11Resource*>(Resource), &_desc,
			ReleaseAndGetAddressOf()), Name.c_str());
	}

	bool ResourceView::Create(const ID3D11ShaderResourceView* View) noexcept(true)
	{
		if (!View)
			return false;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		auto rv = const_cast<ID3D11ShaderResourceView*>(View);
		rv->GetDesc(&srvDesc);

		ComPtr<ID3D11Resource> Res;
		rv->GetResource(Res.GetAddressOf());

		return Create(Res.Get(), &srvDesc);
	}

	void ResourceView::InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true)
	{
		if ((Type & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
		{
			DeviceContext->PSSetShaderResources(BindID, 1, GetAddressOf());
			BindId[0] = BindID;
		}

		if ((Type & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
		{
			DeviceContext->VSSetShaderResources(BindID, 1, GetAddressOf());
			BindId[1] = BindID;
		}

		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
		{
			DeviceContext->CSSetShaderResources(BindID, 1, GetAddressOf());
			BindId[2] = BindID;
		}

		IPType = Type;
	}

	void ResourceView::ShutdownPipeline() noexcept(true)
	{
		if ((IPType & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
			DeviceContext->PSSetShaderResources(BindId[0], 0, nullptr);

		if ((IPType & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
			DeviceContext->VSSetShaderResources(BindId[1], 0, nullptr);

		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetShaderResources(BindId[2], 0, nullptr);

		IPType = 0;
	}

	/// =====================================

	UnorderedAccessView::UnorderedAccessView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), IPType(0), Counts(0), BindId(0)
	{}

	void UnorderedAccessView::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Unordered access \"%s\" info: Format(%X) Buffer([%u:%u] Flags:%X) ViewDimension(%X)",
			Name.c_str(), _desc.Format, _desc.Buffer.FirstElement, _desc.Buffer.NumElements, _desc.Buffer.Flags, _desc.ViewDimension);
	}

	bool UnorderedAccessView::Create(const ID3D11Resource* Resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* Desc) noexcept(true)
	{
		if (!Resource || !Desc)
			return false;

		memcpy(&_desc, Desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		return CheckHrResult(Device->CreateUnorderedAccessView(const_cast<ID3D11Resource*>(Resource), &_desc,
			ReleaseAndGetAddressOf()), Name.c_str());
	}

	bool UnorderedAccessView::Create(const ID3D11UnorderedAccessView* View) noexcept(true)
	{
		if (!View)
			return false;

		D3D11_UNORDERED_ACCESS_VIEW_DESC srvDesc;
		auto rtv = const_cast<ID3D11UnorderedAccessView*>(View);
		rtv->GetDesc(&srvDesc);

		ComPtr<ID3D11Resource> Res;
		rtv->GetResource(Res.GetAddressOf());

		return Create(Res.Get(), &srvDesc);
	}

	void UnorderedAccessView::InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true)
	{
		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
		{
			DeviceContext->CSSetUnorderedAccessViews(BindID, 1, GetAddressOf(), &Counts);
			BindId = BindID;
		}

		IPType = Type;
	}

	void UnorderedAccessView::ShutdownPipeline() noexcept(true)
	{
		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetUnorderedAccessViews(BindId, 0, nullptr, nullptr);

		IPType = 0;
	}

	/// =====================================

	SamplerState::SamplerState(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectShader(Name, Device, DeviceContext), IPType(0)
	{}

	void SamplerState::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Sampler \"%s\" info: Address(%Xx%Xx%X) BorderColor(%.3f,%.3f,%.3f,%.3f) ComparisonFunc(%X)"
			" Filter(%X) MaxAnisotropy(%u) LOD(%.3f:%.3f) MipLODBias(%.3f)", Name.c_str(),
			_desc.AddressU, _desc.AddressV, _desc.AddressW, _desc.BorderColor[0], _desc.BorderColor[1], 
			_desc.BorderColor[2], _desc.BorderColor[3], _desc.ComparisonFunc, _desc.Filter, _desc.MaxAnisotropy,
			_desc.MinLOD, _desc.MaxLOD, _desc.MipLODBias);
	}

	bool SamplerState::Create(const D3D11_SAMPLER_DESC* Desc) noexcept(true)
	{
		if (!Desc)
			return false;

		memcpy(&_desc, Desc, sizeof(D3D11_SAMPLER_DESC));
		return CheckHrResult(Device->CreateSamplerState(&_desc, ReleaseAndGetAddressOf()), Name.c_str());
	}

	void SamplerState::InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true)
	{
		if ((Type & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
		{
			DeviceContext->PSSetSamplers(BindID, 1, GetAddressOf());
			BindId[0] = BindID;
		}

		if ((Type & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
		{
			DeviceContext->VSSetSamplers(BindID, 1, GetAddressOf());
			BindId[1] = BindID;
		}

		if ((Type & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
		{
			DeviceContext->CSSetSamplers(BindID, 1, GetAddressOf());
			BindId[2] = BindID;
		}

		IPType = Type;
	}

	void SamplerState::ShutdownPipeline() noexcept(true)
	{
		if ((IPType & XCELL_PIXEL_SHADER) == XCELL_PIXEL_SHADER)
			DeviceContext->PSSetSamplers(BindId[0], 0, nullptr);

		if ((IPType & XCELL_VERTEX_SHADER) == XCELL_VERTEX_SHADER)
			DeviceContext->VSSetSamplers(BindId[1], 0, nullptr);

		if ((IPType & XCELL_COMPUTE_SHADER) == XCELL_COMPUTE_SHADER)
			DeviceContext->CSSetSamplers(BindId[2], 0, nullptr);

		IPType = 0;
	}

	/// =====================================

	RenderTargetView::RenderTargetView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectDXCustom(Name, Device, DeviceContext)
	{}

	void RenderTargetView::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Render target \"%s\" info: Format(%X) Buffer([%u:%u] Off:%u Width:%u) ViewDimension(%X)", Name.c_str(),
			_desc.Format, _desc.Buffer.FirstElement, _desc.Buffer.NumElements, _desc.Buffer.ElementOffset, _desc.Buffer.ElementWidth,
			_desc.ViewDimension);
	}

	bool RenderTargetView::Create(const ID3D11Resource* Resource, const D3D11_RENDER_TARGET_VIEW_DESC* Desc) noexcept(true)
	{
		if (!Resource || !Desc)
			return false;

		memcpy(&_desc, Desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		return CheckHrResult(Device->CreateRenderTargetView(const_cast<ID3D11Resource*>(Resource), &_desc,
			ReleaseAndGetAddressOf()), Name.c_str());
	}

	bool RenderTargetView::Create(const ID3D11RenderTargetView* View) noexcept(true)
	{
		if (!View)
			return false;

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		auto rtv = const_cast<ID3D11RenderTargetView*>(View);	
		rtv->GetDesc(&rtvDesc);

		ComPtr<ID3D11Resource> Res;
		rtv->GetResource(Res.GetAddressOf());

		return Create(Res.Get(), &rtvDesc);
	}

	/// =====================================

	DepthStencilView::DepthStencilView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		ObjectDXCustom(Name, Device, DeviceContext)
	{}

	void DepthStencilView::DebugInfo() const noexcept(true)
	{
		_MESSAGE("[DBG] Depth and stencil \"%s\" info: Format(%X) Flags(%X) ViewDimension(%X)",
			Name.c_str(), _desc.Format, _desc.Flags, _desc.ViewDimension);
	}

	bool DepthStencilView::Create(const ID3D11Resource* Resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* Desc) noexcept(true)
	{
		if (!Resource || !Desc)
			return false;

		memcpy(&_desc, Desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		return CheckHrResult(Device->CreateDepthStencilView(const_cast<ID3D11Resource*>(Resource), &_desc,
			ReleaseAndGetAddressOf()), Name.c_str());
	}

	bool DepthStencilView::Create(const ID3D11DepthStencilView* View) noexcept(true)
	{
		if (!View)
			return false;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		auto rtv = const_cast<ID3D11DepthStencilView*>(View);
		rtv->GetDesc(&dsvDesc);

		ComPtr<ID3D11Resource> Res;
		rtv->GetResource(Res.GetAddressOf());

		return Create(Res.Get(), &dsvDesc);
	}
}