// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <f4se_common/f4se_version.h>
#include "..\version\resource_version2.h"

#define AUTHOR "perchik71"
#define MODNAME "x-cell"
#define MODVER MAKE_EXE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD)

namespace XCell
{
	static constexpr char Author[] = AUTHOR;
	static constexpr char ModName[] = MODNAME;
	static constexpr uint32_t ModVersion = MODVER;
}

#define FO4_163_VERSION 10163
#define FO4_984_VERSION 10984
#define FO4_191_VERSION 11191

#ifdef FO4_V163
#undef FO4_VER
#define FO4_VER FO4_163_VERSION
#endif // !FO4_V163

#ifdef FO4_V984
#undef FO4_VER
#define FO4_VER FO4_984_VERSION
#endif // !FO4_V984

#ifdef FO4_V191
#undef FO4_VER
#define FO4_VER FO4_191_VERSION
#endif // !FO4_V191