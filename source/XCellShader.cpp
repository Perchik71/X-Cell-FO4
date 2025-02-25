// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellShader.h"
#include <XCellPlugin.h>

namespace XCell
{
	extern HMODULE gModuleHandle;

	Shader::Shader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Object(Name), Device(Device), DeviceContext(DeviceContext)
	{}

	Shader::~Shader()
	{
		Release();
	}

	bool Shader::LoadFromResource(UInt32 ResourceID) noexcept(true)
	{
		return LoadFromResource(MAKEINTRESOURCEA(ResourceID));
	}

	bool Shader::LoadFromResource(const char* ResourceName) noexcept(true)
	{
		if (!ResourceName)
			return false;

		HRSRC hResource = FindResourceA(gModuleHandle, ResourceName, "SHADER");
		if (!hResource)
		{
			_WARNING("Resource no found");
			return false;
		}

		UInt32 Size = SizeofResource(gModuleHandle, hResource);
		if (Size >= numeric_limits<int32_t>::max())
		{
			_WARNING("Size of resource exceeds 2 gigs");
			return false;
		}

		auto hResourceMemory = LoadResource(gModuleHandle, hResource);
		if (!hResourceMemory) 
		{
			_ERROR("hResourceMemory == nullptr");
			return false;
		}

		StreamBinary.WriteBuf(LockResource(hResourceMemory), (int32_t)Size);
		StreamBinary.SetPosition(0);

		bool Successed = Size == StreamBinary.Size;
		if (!Successed)
		{
			StreamBinary.Clear();
			_ERROR("An error occurred while reading the resource");
		}
		else
			_MESSAGE("Resource was loaded successfully");

		FreeResource(hResourceMemory);
		return Successed;
	}
}