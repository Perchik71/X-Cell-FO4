// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se_common/f4se_version.h>
#include "..\version\resource_version2.h"

#define AUTHOR "perchik71"
#define MODNAME "x-cell"
#define MODVER MAKE_EXE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD)

namespace xc
{
	static constexpr char author[] = AUTHOR;
	static constexpr char modname[] = MODNAME;
	static constexpr uint32_t modver = MODVER;
}