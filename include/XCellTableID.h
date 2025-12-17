// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <unordered_map>
#include <initializer_list>

namespace XCell
{
	namespace REL
	{
		class TableID
		{
			UInt32 _version;
			unordered_map<UInt32, UInt32> _items;
		public:
			TableID(UInt32 Version, const initializer_list<pair<UInt32, UInt32>>& list);
			virtual ~TableID() = default;

			[[nodiscard]] inline virtual UInt32 GetVersion() const noexcept(true) { return _version; }

			[[nodiscard]] virtual UInt32 At(UInt32 Index) const noexcept(true);
			[[nodiscard]] virtual UInt32 Nearest(UInt32 Index) const noexcept(true);
			[[nodiscard]] virtual UInt64 ID(UInt32 Index) const noexcept(true);

			[[nodiscard]] inline UInt32 operator[](UInt32 Index) const noexcept(true) { return At(Index); }

			XCPropertyReadOnly(GetVersion) UInt32 Version;
		};

		UInt32 __stdcall Nearest(UInt32 Index);
		UInt64 __stdcall ID(UInt32 Index);
		UInt32 __stdcall Version();

		extern TableID K_163;
		extern TableID K_984;
		extern TableID K_191;
		extern TableID* K_CUR;
	}
}