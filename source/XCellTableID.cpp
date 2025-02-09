// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/PluginAPI.h>

#include "XCellTableID.h"
#include "XCellRelocator.h"

namespace XCell
{
	namespace REL
	{
		TableID::TableID(UInt32 Version, const initializer_list<pair<UInt32, UInt32>>& list) :
			_version(Version)
		{
			for (auto& It : list)
				_items.insert(make_pair(It.first, It.second));
		}

		UInt32 TableID::At(UInt32 Index) const noexcept(true)
		{
			auto It = _items.lower_bound(Index);
			return (It == _items.end() || abs((long long)It->first - Index) != 0) ? 0 : It->second;
		}

		UInt32 TableID::Nearest(UInt32 Index) const noexcept(true)
		{
			auto It = _items.lower_bound(Index);
			return (It == _items.end() || abs((long long)It->first - Index) > 10) ? 0 : It->second;
		}

		UInt64 TableID::ID(UInt32 Index) const noexcept(true)
		{
			auto ID = At(Index);
			return ID ? Offset(ID) : 0;
		}

		TableID* K_CUR = nullptr;

		UInt32 __stdcall Nearest(UInt32 Index)
		{
			return K_CUR ? K_CUR->Nearest(Index) : 0;
		}

		UInt64 __stdcall ID(UInt32 Index)
		{
			return K_CUR ? K_CUR->ID(Index) : 0;
		}

		UInt32 __stdcall Version()
		{
			return K_CUR ? K_CUR->Version : 0;
		}

		TableID K_163(RUNTIME_VERSION_1_10_163, {
			// LISTENER DX11
			{ 0,		0x1D18A57 },
			{ 10,		0x61E0910 },
			{ 20,		0x1D4FE40 },
			{ 21,		0x2043A80 },

			// MEMORY
			{ 30,		0x1B0EFD0 },
			{ 40,		0x1B0F2E0 },
			{ 50,		0x1B0E7D0 },
			{ 60,		0x1B13F70 },
			{ 70,		0x1B14580 },
			{ 80,		0x1E21B10 },
			{ 90,		0x211214B },
			{ 100,		0x2112151 },
			{ 110,		0xD0C160 },
			{ 120,		0x1B0EDB0 },
			{ 130,		0x1B13DF0 },
			{ 140,		0x1B14740 },
			{ 150,		0x2EB92C8 },

			// LIBDEFLATE
			{ 160,		0x13267D },
			{ 165,		0x1326AF },

			// LOADSCREEN
			{ 170,		0x12989E0 },

			// FACEGEN
			{ 200,		0x5B57F0 },
			{ 205,		0x679910 },
			{ 210,		0x59DADD0 },
			{ 215,		0x679B20 },
			{ 220,		0x679BB2 },
			{ 225,		0x1C97190 },

			// UPSCALER
			//{ 230,		0x1D18A57 },
			//{ 231,		0x1D4FE40 },
			});

		TableID K_984(RUNTIME_VERSION_1_10_984, {
			// LISTENER DX11
			{ 0,		0x16FB147 },
			{ 10,		0x3769610 },
			{ 20,		0x1739EB0 },
			{ 21,		0x1966EC0 },

			// MEMORY
			{ 30,		0x153D7D0 },
			{ 40,		0x153DC30 },
			{ 41,		0x153DA30 },
			{ 50,		0x153CFA0 },
			{ 60,		0x1542010 },
			{ 70,		0x15425E0 },
			{ 80,		0x17D9DF0 },
			{ 90,		0x19FF5D9 },
			{ 100,		0x19FF5E4 },
			{ 110,		0xB8DC50 },
			{ 120,		0x153D5D0 },
			{ 130,		0x1541E90 },
			{ 140,		0x15427A0 },
			{ 150,		0x25131D8 },
			{ 151,		0x153DDA0 },	

			// LIBDEFLATE
			{ 160,		0x2A5352 },
			{ 165,		0x2A5384 },

			// LOADSCREEN
			{ 170,		0xFE23C0 },

			// GREY MOVIES
			{ 180,		0x19FFE89 },

			// PACKAGE ALLOCATE LOCATION
			{ 190,		0x2352E0 },
			{ 191,		0x711CE4 },

			// FACEGEN
			{ 200,		0x601B50 },
			{ 205,		0x68B6F3 },
			{ 210,		0x2F6ED50 },
			{ 215,		0x68B900 },
			{ 220,		0x68B994 },
			{ 225,		0x16894C0 },	

			// UPSCALER
			{ 230,		0x2CDBDA0 },	// Size W : Display
			{ 231,		0x2CDBDB8 },	// Size H : Display
			{ 232,		0x2CDBDD0 },	// Location X : Display
			{ 233,		0x2CDBDE8 },	// Location Y : Display
			{ 240,		0x156C230 },	// First use read ^ data
			
			});
	}
}