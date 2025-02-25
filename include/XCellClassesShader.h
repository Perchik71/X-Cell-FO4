// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include "XCellObject.h"

#include <wrl/client.h>
#include <d3d11.h>

namespace XCell
{
	using namespace Microsoft::WRL;

	enum : UInt32
	{
		XCELL_PIXEL_SHADER = 1 << 0,
		XCELL_VERTEX_SHADER = 1 << 1,
		XCELL_COMPUTE_SHADER = 1 << 2,
	};

	class ObjectDXCustom : public Object
	{
	protected:
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
	public:
		ObjectDXCustom(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~ObjectDXCustom() = default;

		ObjectDXCustom(const ObjectDXCustom&) = delete;
		ObjectDXCustom& operator=(const ObjectDXCustom&) = delete;
	};

	class ObjectShader : public ObjectDXCustom
	{
	public:
		ObjectShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~ObjectShader() = default;

		virtual void InitPipeline(UInt32, UInt32) noexcept(true) = 0;
		virtual void ShutdownPipeline() noexcept(true) = 0;

		ObjectShader(const ObjectShader&) = delete;
		ObjectShader& operator=(const ObjectShader&) = delete;
	};

	class TextureShader : public ObjectDXCustom
	{
		D3D11_TEXTURE2D_DESC _desc;
		ComPtr<ID3D11Texture2D> _texture;
	public:
		TextureShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~TextureShader() = default;

		TextureShader(const TextureShader&) = delete;
		TextureShader& operator=(const TextureShader&) = delete;

		virtual bool SaveTextureToFileAsDDS(const char* FileName) const noexcept(true);
		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const D3D11_TEXTURE2D_DESC* Desc) noexcept(true);
		virtual bool Create(const ID3D11Texture2D* Texture, UInt32 BindFlags = D3D11_BIND_SHADER_RESOURCE) noexcept(true);
		virtual bool Create(const ID3D11Resource* Resource, UInt32 BindFlags = D3D11_BIND_SHADER_RESOURCE) noexcept(true);

		virtual bool CopyFrom(const TextureShader* Texture) noexcept(true);
		virtual bool CopyFrom(const ID3D11Texture2D* Texture) noexcept(true);
		virtual bool CopyFrom(const ID3D11Resource* Resource) noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _texture == nullptr; }
		[[nodiscard]] virtual inline ID3D11Texture2D* Get() const noexcept(true) { return _texture.Get(); }
		[[nodiscard]] virtual inline ID3D11Texture2D** GetAddressOf() noexcept(true) { return _texture.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11Texture2D** ReleaseAndGetAddressOf() noexcept(true) { return _texture.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_TEXTURE2D_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class BufferShader : public ObjectShader
	{
		UINT IPType, BindId[3];
		D3D11_BUFFER_DESC _desc;
		ComPtr<ID3D11Buffer> _Buffer;
	public:
		BufferShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~BufferShader() = default;

		BufferShader(const BufferShader&) = delete;
		BufferShader& operator=(const BufferShader&) = delete;

		virtual bool Create(const void* Buffer, UInt32 Size, UInt32 BindFlags, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT) noexcept(true);
		virtual void InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true);
		virtual void ShutdownPipeline() noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _Buffer == nullptr; }
		[[nodiscard]] virtual ID3D11Buffer* Get() const noexcept(true) { return _Buffer.Get(); }
		[[nodiscard]] virtual ID3D11Buffer** GetAddressOf() noexcept(true) { return _Buffer.GetAddressOf(); }
		[[nodiscard]] virtual ID3D11Buffer** ReleaseAndGetAddressOf() noexcept(true) { return _Buffer.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_BUFFER_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class ResourceView : public ObjectShader
	{
		UInt32 IPType, BindId[3];
		D3D11_SHADER_RESOURCE_VIEW_DESC _desc;
		ComPtr<ID3D11ShaderResourceView> _resourceView;
	public:
		ResourceView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~ResourceView() = default;

		ResourceView(const ResourceView&) = delete;
		ResourceView& operator=(const ResourceView&) = delete;

		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const ID3D11Resource* Resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* Desc) noexcept(true);
		virtual bool Create(const ID3D11ShaderResourceView* View) noexcept(true);
		virtual void InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true);
		virtual void ShutdownPipeline() noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _resourceView == nullptr; }
		[[nodiscard]] virtual inline ID3D11ShaderResourceView* Get() const noexcept(true) { return _resourceView.Get(); }
		[[nodiscard]] virtual inline ID3D11ShaderResourceView** GetAddressOf() noexcept(true) { return _resourceView.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11ShaderResourceView** ReleaseAndGetAddressOf() noexcept(true) { return _resourceView.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_SHADER_RESOURCE_VIEW_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class UnorderedAccessView : public ObjectShader
	{
		UINT Counts, IPType, BindId;
		D3D11_UNORDERED_ACCESS_VIEW_DESC _desc;
		ComPtr<ID3D11UnorderedAccessView> _View;
	public:
		UnorderedAccessView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~UnorderedAccessView() = default;

		UnorderedAccessView(const UnorderedAccessView&) = delete;
		UnorderedAccessView& operator=(const UnorderedAccessView&) = delete;

		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const ID3D11Resource* Resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* Desc) noexcept(true);
		virtual bool Create(const ID3D11UnorderedAccessView* View) noexcept(true);
		virtual void InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true);
		virtual void ShutdownPipeline() noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _View == nullptr; }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView* Get() const noexcept(true) { return _View.Get(); }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView** GetAddressOf() noexcept(true) { return _View.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11UnorderedAccessView** ReleaseAndGetAddressOf() noexcept(true) { return _View.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_UNORDERED_ACCESS_VIEW_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class SamplerState : public ObjectShader
	{
		UInt32 IPType, BindId[3];
		D3D11_SAMPLER_DESC _desc;
		ComPtr<ID3D11SamplerState> _samplerState;
	public:
		SamplerState(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~SamplerState() = default;

		SamplerState(const SamplerState&) = delete;
		SamplerState& operator=(const SamplerState&) = delete;

		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const D3D11_SAMPLER_DESC* Desc) noexcept(true);
		virtual void InitPipeline(UInt32 Type, UInt32 BindID) noexcept(true);
		virtual void ShutdownPipeline() noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _samplerState == nullptr; }
		[[nodiscard]] virtual inline ID3D11SamplerState* Get() const noexcept(true) { return _samplerState.Get(); }
		[[nodiscard]] virtual inline ID3D11SamplerState** GetAddressOf() noexcept(true) { return _samplerState.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11SamplerState** ReleaseAndGetAddressOf() noexcept(true) { return _samplerState.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_SAMPLER_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class RenderTargetView : public ObjectDXCustom
	{
		D3D11_RENDER_TARGET_VIEW_DESC _desc;
		ComPtr<ID3D11RenderTargetView> _renderTarget;
	public:
		RenderTargetView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~RenderTargetView() = default;

		RenderTargetView(const RenderTargetView&) = delete;
		RenderTargetView& operator=(const RenderTargetView&) = delete;

		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const ID3D11Resource* Resource, const D3D11_RENDER_TARGET_VIEW_DESC* Desc) noexcept(true);
		virtual bool Create(const ID3D11RenderTargetView* View) noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _renderTarget == nullptr; }
		[[nodiscard]] virtual inline ID3D11RenderTargetView* Get() const noexcept(true) { return _renderTarget.Get(); }
		[[nodiscard]] virtual inline ID3D11RenderTargetView** GetAddressOf() noexcept(true) { return _renderTarget.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11RenderTargetView** ReleaseAndGetAddressOf() noexcept(true) { return _renderTarget.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_RENDER_TARGET_VIEW_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};

	class DepthStencilView : public ObjectDXCustom
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC _desc;
		ComPtr<ID3D11DepthStencilView> _depthTarget;
	public:
		DepthStencilView(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~DepthStencilView() = default;

		DepthStencilView(const DepthStencilView&) = delete;
		DepthStencilView& operator=(const DepthStencilView&) = delete;

		virtual void DebugInfo() const noexcept(true);
		virtual bool Create(const ID3D11Resource* Resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* Desc) noexcept(true);
		virtual bool Create(const ID3D11DepthStencilView* View) noexcept(true);

		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return _depthTarget == nullptr; }
		[[nodiscard]] virtual inline ID3D11DepthStencilView* Get() const noexcept(true) { return _depthTarget.Get(); }
		[[nodiscard]] virtual inline ID3D11DepthStencilView** GetAddressOf() noexcept(true) { return _depthTarget.GetAddressOf(); }
		[[nodiscard]] virtual inline ID3D11DepthStencilView** ReleaseAndGetAddressOf() noexcept(true) { return _depthTarget.ReleaseAndGetAddressOf(); }
		[[nodiscard]] virtual inline const D3D11_DEPTH_STENCIL_VIEW_DESC* GetDesc() const noexcept(true) { return &_desc; }
	};
}