// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellObject.h"
#include "XCellStringUtils.h"

namespace XCell
{
	Object::Object(const char* Name) :
		_name(Name)
	{}

	UInt32 Object::GetNameHash() const noexcept(true)
	{
		if (_name.empty())
			return 0xFFFFFFFFul;

		auto s = strdup(_name.c_str());
		if (!s)
			return 0xFFFFFFFFul;
		
		auto h = Utils::MurmurHash32A(_strlwr(s), _name.length(), 0);
		free(s);

		return h;
	}

	UInt64 Object::GetNameHash64() const noexcept(true)
	{
		if (_name.empty())
			return 0xFFFFFFFFul;

		auto s = strdup(_name.c_str());
		if (!s)
			return 0xFFFFFFFFul;

		auto h = Utils::MurmurHash64A(_strlwr(s), _name.length(), 0);
		free(s);

		return h;
	}

	Object::Object(const Object& Rhs) :
		_name(Rhs._name)
	{}

	Object& Object::operator=(const Object& Rhs)
	{
		_name = Rhs._name;
		return *this;
	}
}