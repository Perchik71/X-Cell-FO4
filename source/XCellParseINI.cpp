// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellParseINI.h"

#include "XCellStream.h"
#include "XCellStringUtils.h"

namespace XCell
{
	// OptionINI

	OptionINI::OptionINI(const char* Name, const char* Value) :
		Object(Name), _value(Value)
	{}

	OptionINI::OptionINI(const OptionINI& Rhs) :
		Object(Rhs), _value(Rhs._value)
	{}

	UInt32 OptionINI::AsInteger() const noexcept(true) 
	{
		auto s = AsStringWithoutQuote();
		Utils::Trim(s);
		if (s.find_first_of("0x") == 0)
			return (UInt32)strtoul(s.c_str(), nullptr, 16);
		return (UInt32)strtoul(s.c_str(), nullptr, 10);
	}

	string OptionINI::AsStringWithoutQuote() const noexcept(true)
	{
		if (_value.empty())
			return _value;

		if (_value[0] == '"')
			return _value.substr(1, _value.length() - 2);

		return _value;
	}

	void OptionINI::SetInteger(long Value) noexcept(true) 
	{
		char buf[_MAX_ITOSTR_BASE10_COUNT] = { 0 };
		_itoa_s(Value, buf, 10);
		_value = buf; 
	}

	// SectionINI

	SectionINI::SectionINI(const char* Name) :
		Object(Name)
	{}

	bool SectionINI::Contains(const char* Name) const noexcept(true)
	{
		return _options.find(Object(Name).NameHash) != _options.end();
	}

	OptionINI& SectionINI::At(const char* Name) noexcept(true)
	{
		auto hash = Object(Name).NameHash;
		auto it = _options.find(hash);
		if (it == _options.end())
		{
			auto pair = make_pair(hash, make_shared<OptionINI>(Name, ""));
			_options.insert(pair);
			return *pair.second;
		}
		return *(it->second);
	}

	bool SectionINI::Remove(const char* Name) noexcept(true)
	{
		auto hash = Object(Name).NameHash;
		auto it = _options.find(hash);
		if (it != _options.end())
		{
			_options.erase(it);
			return true;
		}
		return false;
	}

	// DataINI

	bool DataINI::Contains(const char* Name) const noexcept(true)
	{
		return _items.find(Object(Name).NameHash) != _items.end();
	}

	SectionINI& DataINI::At(const char* Name) noexcept(true)
	{
		auto hash = Object(Name).NameHash;
		auto it = _items.find(hash);
		if (it == _items.end())
		{
			auto pair = make_pair(hash, make_shared<SectionINI>(Name));
			_items.insert(pair);
			return *pair.second;
		}
		return *(it->second);
	}

	// ParseINI

	ParseINI::ParseINI() :
		DataINI()
	{}

	bool ParseINI::Parse(const char* FileName)
	{
		IScopedCriticalSection Locker(&_section);

		TextFileStream Stream;
		if (!Stream.Open(FileName, FileStreamMode::kStreamOpenRead))
			return false;

		// No supported comments in line with options/sections

		string current_line_utf8;
		string current_section;
		string value_option;
		string name_option;

		do
		{
			if (Stream.ReadStringLine(current_line_utf8, kTextEncode_UTF8) < 0)
				break;

			Utils::Trim(current_line_utf8);

			auto first_char = current_line_utf8[0];
			switch ((uint16_t)first_char)
			{
				// Skips comments
			case ';':
			case '#':
				break;
			case '[':
				current_section = current_line_utf8.substr(1, current_line_utf8.length() - 2);
				break;
			default:

				// get option name
				auto sep = current_line_utf8.find_first_of("=:");
				if (sep == string::npos) break;
				name_option = current_line_utf8.substr(0, sep);
				Utils::Trim(name_option);

				// get option value (supported \\)
				value_option = current_line_utf8.substr(sep + 1);
				Utils::Trim(value_option);

				// WinAPI no supported
				//do
				//{
				//	// continue with new line
				//	if (value_option[value_option.length() - 1] == L'\\')
				//	{
				//		value_option.erase(value_option.length() - 1);
				//		Utils::Trim(value_option);

				//		if (Stream.ReadStringLine(current_line_utf8, kTextEncode_UTF8) <= 0)
				//			break;

				//		value_option.append(Utils::Trim(current_line_utf8));
				//	}
				//	else break;
				//} while (1);

				if (!current_section.empty() && !name_option.empty())
					At(current_section.c_str())[name_option.c_str()].SetString(value_option.c_str());
				else
					_WARNING("The option has an incorrect name \"%s:%s\".",
						(name_option.empty() ? "" : name_option.c_str()),
						(current_section.empty() ? "" : current_section.c_str()));

				break;
			}

		} while (!Stream.Eof());

		Stream.Close();

		return true;
	}

	bool WriteINI::Write(const char* FileName, DataINI& Data) const noexcept(true)
	{
		IScopedCriticalSection Locker(&(const_cast<WriteINI*>(this)->_section));

		TextFileStream Stream;
		if (!Stream.Open(FileName, FileStreamMode::kStreamCreate))
			return false;

		for (auto& Section : Data)
		{
			if (!Section.second->Count())
				continue;

			Stream.WriteFormatString("[%s]\n", Section.second->Name.c_str());

			for (auto& Option : *Section.second)
			{
				auto Name = Option.second->Name;
				auto Value = Option.second->AsString();

				if (Name.empty())
					continue;

				if (Value.empty())
					Stream.WriteFormatString("%s=\n", Name.c_str());
				else
					Stream.WriteFormatString("%s=%s\n", Name.c_str(), Value.c_str());
			}

			Stream.WriteString("\n");
		}

		Stream.Close();

		return true;
	}
}