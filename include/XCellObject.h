// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

namespace XCell
{
	class Object
	{
		string _name;
	public:
		Object(const char* Name);
		virtual ~Object() = default;

		virtual inline const string GetName() const noexcept(true) { return _name; }
		virtual UInt32 GetNameHash() const noexcept(true);
		virtual UInt64 GetNameHash64() const noexcept(true);

		XCPropertyReadOnly(GetName) const string Name;
		XCPropertyReadOnly(GetNameHash) UInt32 NameHash;
		XCPropertyReadOnly(GetNameHash64) UInt64 NameHash64;

		Object(const Object& Rhs);
		Object& operator=(const Object& Rhs);
	};
}