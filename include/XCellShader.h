// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <XCellObject.h>
#include <XCellStream.h>

#include <d3d11.h>

namespace XCell
{
	class Shader : public Object
	{
	protected:
		MemoryStream StreamBinary;
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
	public:
		Shader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
		virtual ~Shader();

		virtual void Bind() noexcept(true) = 0;
		virtual void Unbind() noexcept(true) = 0;
		virtual bool Install() noexcept(true) = 0;
		virtual void Shutdown() noexcept(true) = 0;
		virtual inline void Release() noexcept(true) { Shutdown(); }

		[[nodiscard]] virtual bool LoadFromResource(UInt32 ResourceID) noexcept(true);
		[[nodiscard]] virtual bool LoadFromResource(const char* ResourceName) noexcept(true);
		[[nodiscard]] virtual inline bool IsEmpty() const noexcept(true) { return StreamBinary.IsEmpty(); }

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
	};
}