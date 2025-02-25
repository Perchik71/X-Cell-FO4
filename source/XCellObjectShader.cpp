// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellObjectShader.h"

#include <comdef.h>

namespace XCell
{
	ObjectShader::ObjectShader(const char* Name, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) :
		Object(Name), Device(Device), DeviceContext(DeviceContext)
	{}
}