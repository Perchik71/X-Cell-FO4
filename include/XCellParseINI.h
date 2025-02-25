// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <ICriticalSection.h>

#include <unordered_map>
#include <memory>

#include "XCellObject.h"

namespace XCell
{
	class OptionINI : public Object
	{
		string _value;
	public:
		OptionINI(const char* Name, const char* Value);
		OptionINI(const OptionINI& Rhs);

		[[nodiscard]] UInt32 AsInteger() const noexcept(true);
		[[nodiscard]] virtual string AsStringWithoutQuote() const noexcept(true);
		[[nodiscard]] inline virtual string AsString() const noexcept(true) { return _value; }
		virtual void SetInteger(long Value) noexcept(true);
		inline virtual void SetString(const char* Value) noexcept(true) { _value = Value; }

		OptionINI& operator=(const OptionINI&) = delete;
	};

	class SectionINI : public Object
	{
		unordered_map<UInt32, shared_ptr<OptionINI>> _options;
	public:
		SectionINI(const char* Name);

		[[nodiscard]] virtual bool Contains(const char* Name) const noexcept(true);
		virtual OptionINI& At(const char* Name) noexcept(true);
		[[nodiscard]] inline virtual UInt64 Count() const noexcept(true) { return (UInt64)_options.size(); }
		inline virtual void Clear() noexcept(true) { _options.clear(); }
		virtual bool Remove(const char* Name) noexcept(true);

		inline OptionINI& operator[](const char* Name) noexcept(true) { return At(Name); }

		/// need for STL
		inline unordered_map<UInt32, shared_ptr<OptionINI>>::iterator begin() noexcept(true) { return _options.begin(); }
		inline unordered_map<UInt32, shared_ptr<OptionINI>>::iterator end() noexcept(true) { return _options.end(); }
		inline unordered_map<UInt32, shared_ptr<OptionINI>>::const_iterator cbegin() const noexcept(true) { return _options.cbegin(); }
		inline unordered_map<UInt32, shared_ptr<OptionINI>>::const_iterator cend() const noexcept(true) { return _options.cend(); }

		SectionINI(const SectionINI&) = delete;
		SectionINI& operator=(const SectionINI&) = delete;
	};

	class DataINI
	{
		bool _need_save;
		unordered_map<UInt32, shared_ptr<SectionINI>> _items;
	public:
		DataINI() = default;
		virtual ~DataINI() = default;

		virtual bool Contains(const char* Name) const noexcept(true);
		virtual SectionINI& At(const char* Name) noexcept(true);
		[[nodiscard]] inline virtual UInt64 Count() const noexcept(true) { return (UInt64)_items.size(); }
		inline virtual void Clear() noexcept(true) { _items.clear(); }

		inline SectionINI& operator[](const char* Name) noexcept(true) { return At(Name); }

		inline bool HasChanged() const noexcept(true) { return _need_save; }
		inline void SetChanged(bool Changed) noexcept(true) { _need_save = Changed; }

		/// need for STL
		inline unordered_map<UInt32, shared_ptr<SectionINI>>::iterator begin() noexcept(true) { return _items.begin(); }
		inline unordered_map<UInt32, shared_ptr<SectionINI>>::iterator end() noexcept(true) { return _items.end(); }
		inline unordered_map<UInt32, shared_ptr<SectionINI>>::const_iterator cbegin() const noexcept(true) { return _items.cbegin(); }
		inline unordered_map<UInt32, shared_ptr<SectionINI>>::const_iterator cend() const noexcept(true) { return _items.cend(); }

		DataINI(const DataINI&) = delete;
		DataINI& operator=(const DataINI&) = delete;
	};

	class ParseINI : public DataINI
	{
		ICriticalSection _section;
		unordered_map<UInt32, shared_ptr<SectionINI>> _items;
	public:
		ParseINI();

		virtual bool Parse(const char* FileName);
	};

	class WriteINI
	{
		ICriticalSection _section;
	public:
		WriteINI() = default;
		virtual ~WriteINI() = default;

		virtual bool Write(const char* FileName, DataINI& Data) const noexcept(true);
	};
}