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
			auto It = _items.find(Index);
			return It == _items.end() ? 0 : It->second;
		}

		UInt32 TableID::Nearest(UInt32 Index) const noexcept(true)
		{
			if (!_items.size())
				return 0;

			auto NearestIt = _items.begin();
			for (auto It = _items.begin(); It != _items.end(); It++)
			{
				if (It->first > Index)
					NearestIt = It;
				else
					break;
			}
			
			return (NearestIt == _items.end() || abs((long long)NearestIt->first - Index) > 10) ? 0 : NearestIt->second;
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
			{ 21,		0xAE6C70 },		// 0x2043A80
			{ 22,		0x28571FC },

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
			{ 230,		0x384FBF0 },	// Size W : Display
			{ 231,		0x384FC08 },	// Size H : Display
			{ 232,		0x384FC20 },	// Location X : Display
			{ 233,		0x384FC38 },	// Location Y : Display
			{ 240,		0x1B4EC90 },	// Before first use read ^ data
			{ 245,		0x38C51B0 },	// TAA or FXAA

			// INIT TINTS
			{ 250,		0x5AF94D },
			{ 251,		0x5AFB60 },
			{ 252,		0x292BBE0 },
			{ 253,		0x11E4F1 },
			{ 254,		0x5BDC49 },
			{ 255,		0x120504 },

			// LOD
			{ 260,		0x3F4C6C },
			{ 261,		0x3F4C90 },
			{ 262,		0x4033C0 },
			{ 263,		0x40F37B },
			{ 264,		0x40F3DC },
			{ 265,		0x3DF32B },
			{ 266,		0x7A859C },
			{ 267,		0x52A80 },
			// 2F3BE2	??? Keyword?
			// 42B380
			});

		TableID K_984(RUNTIME_VERSION_1_10_984, {
			// LISTENER DX11
			{ 0,		0x16FB147 }, 
			{ 10,		0x3769610 },
			{ 20,		0x1739EB0 },
			{ 21,		0x9E7A20 }, // 9E8EF0 // 1966EC0 -- before cursor
			{ 22,		0x209D130 },

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
			{ 240,		0x156C230 },	// Before first use read ^ data
			{ 245,		0x2D25AA0 },	// TAA or FXAA
			
			// INIT TINTS
			{ 250,		0x5FBDFB },
			{ 251,		0x5FBBB4 },
			{ 252,		0x2156D30 },
			{ 253,		0x29B434 },
			{ 254,		0x60A083 },

			// LOD
			{ 260,		0x4A7830 },
			{ 261,		0x4A7860 },
			{ 262,		0x4B6C40 },
			{ 263,		0x4C310B },
			{ 264,		0x4C316C },
			{ 265,		0x3DF32B },
			{ 266,		0x4A7890 },

			// EXTRA COUNT
			{ 270,		0x226F90 },
			{ 271,		0x24F8AB },
			{ 272,		0x21ED17 },
			{ 273,		0x4ADA60 },
			{ 274,		0xFD662A },
			{ 275,		0x58D412 },
			{ 276,		0x50F8ED },
			{ 277,		0x2240ED },
			{ 278,		0x226F27 },
			{ 279,		0x23ED2E },
			{ 280,		0x226F39 },
			{ 281,		0x226F53 },
			{ 282,		0x22BAD0 },
			{ 283,		0xCE3E49 },
			{ 284,		0x10FBB91 },
			{ 285,		0x10FB781 },
			{ 286,		0x40470C },
			{ 287,		0x3F18D1 },
			{ 288,		0x3F18BB },
			{ 289,		0x4AEF51 },

			});

		TableID K_191(RUNTIME_VERSION_1_11_191, {
			// LISTENER DX11
			{ 0,		0x1815E22 },
			{ 10,		0x3A0F410 },
			{ 20,		0x1854BA0 },
			{ 21,		0xA3B300 },
			{ 22,		0x21F27C0 },

			// MEMORY
			{ 30,		0x16579C0 },
			{ 40,		0x1657E20 },
			{ 41,		0x1657C20 },
			{ 50,		0x1657190 },
			{ 60,		0x165BE20 },
			{ 70,		0x165C3F0 },
			{ 80,		0x18F4AE0 },
			{ 90,		0x1B1A4D7 },
			{ 100,		0x1B1A4E2 },
			{ 110,		0xC13680 },
			{ 120,		0x16577C0 },
			{ 130,		0x165BC90 },
			{ 140,		0x165C5B0 },
			{ 150,		0x27207D8 },
			{ 151,		0x1657F90 },

			// LIBDEFLATE
			{ 160,		0x2F9632 },
			{ 165,		0x2F9664 },

			// LOADSCREEN
			{ 170,		0x1068070 },

			// GREY MOVIES
			{ 180,		0x1B1AE69 },

			// PACKAGE ALLOCATE LOCATION
			{ 190,		0x2895F0 },
			{ 191,		0x765424 },

			// FACEGEN
			{ 200,		0x6554E0 },
			{ 205,		0x6DEE53 },
			{ 210,		0x31E61B0 },
			{ 215,		0x6DF060 },
			{ 220,		0x6DF0F4 },
			{ 225,		0x17A4120 },

			// UPSCALER
			{ 230,		0x2F425E0 },	// Size W : Display
			{ 231,		0x2F425F8 },	// Size H : Display
			{ 232,		0x2F42610 },	// Location X : Display
			{ 233,		0x2F42628 },	// Location Y : Display
			{ 240,		0x1686520 },	// Before first use read ^ data
			{ 245,		0x2F8C5B8 },	// TAA or FXAA

			// INIT TINTS
			{ 250,		0x64F78B },
			{ 251,		0x64F544 },
			{ 252,		0x22ACD20 },
			{ 253,		0x2EF647 },
			{ 254,		0x65DA13 },

			// LOD
			{ 260,		0x4FB760 },
			{ 261,		0x4FB790 },
			{ 262,		0x50AB60 },
			{ 263,		0x51708B },
			{ 264,		0x5170EC },
			{ 265,		0x43314B },
			{ 266,		0x4FB7C0 },

			// EXTRA COUNT
			{ 270,		0x27B2A0 },
			{ 271,		0x2A3BAB },
			{ 272,		0x273027 },
			{ 273,		0x501980 },
			{ 274,		0x105C40A },
			{ 275,		0x5E1392 },
			{ 276,		0x56386D },
			{ 277,		0x2783FD },
			{ 278,		0x27B237 },
			{ 279,		0x29303E },
			{ 280,		0x27B249 },
			{ 281,		0x27B263 },
			{ 282,		0x27FDE0 },
			{ 283,		0xD69959 },
			{ 284,		0x11815D1 },
			{ 285,		0x11811C1 },
			{ 286,		0x45852C },
			{ 287,		0x4456F1 },
			{ 288,		0x4456DB },
			{ 289,		0x502E71 },

			// 29305E << new need test
			});
	}
}