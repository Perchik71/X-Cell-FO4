// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// F4SE
#include <ICriticalSection.h>

#include <unordered_map>
#include <memory>

// XCell
#include "XCellObject.h"

namespace XCell
{
	class Setting : public Object
	{
	public:
		enum Types
		{
			stBool = 0x0,	// 'b' size 1
			stChar = 0x1,	// 'c' size 1
			stUnsignedChar = 0x2,	// 'h' size 1
			stSignedInt = 0x3,	// 'i'/'n' size 4
			stUnsignedInt = 0x4,	// 'u' size 4
			stFloat = 0x5,	// 'f' size 4
			stString = 0x6,	// 'S'/'s' size indet.
			stRGB = 0x7,	// 'r' size 4, alpha byte set to 255
			stRGBA = 0x8,	// 'a' size 4
			stMAX = 0x9,
			stError
		};

		// union for various value types
		union Value
		{
			bool		b;
			char		c;
			uint8_t		h;
			int32_t		i;
			uint32_t	u;
			float		f;
			char* s;
			struct Color {
				// alpha is least sig. byte, *opposite* standard windows order
				uint8_t alpha;
				uint8_t blue;
				uint8_t green;
				uint8_t red;
			} rgba;
		};

		explicit Setting(const char* name, bool value, const char* desc = "");
		explicit Setting(const char* name, char value, const char* desc = "");
		Setting(const char* name, uint8_t value, const char* desc = "");
		Setting(const char* name, int32_t value, const char* desc = "");
		Setting(const char* name, uint32_t value, const char* desc = "");
		Setting(const char* name, float value, const char* desc = "");
		Setting(const char* name, const char* value, const char* desc = "");
		Setting(const char* name, uint8_t r, uint8_t g, uint8_t b, const char* desc = "");
		Setting(const char* name, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const char* desc = "");

		explicit Setting(const char* name, const Value& value, const char* desc = "");
		virtual ~Setting();

		Setting(const Setting&) = delete;
		Setting& operator=(const Setting&) = delete;

		bool operator==(const char* name) const noexcept;
		bool operator!=(const char* name) const noexcept;

		static constexpr uint32_t HashInvalid = 0xFFFFFFFFul;

		[[nodiscard]] virtual Types GetValueType() const noexcept;

		virtual void ClearValue();
		[[nodiscard]] virtual bool EmptyValue() const noexcept;

		virtual bool SetBool(bool value);
		virtual bool SetChar(char value);
		virtual bool SetChar(wchar_t value);
		virtual bool SetUnsignedChar(uint8_t value);
		virtual bool SetSignedInt(int32_t value);
		virtual bool SetUnsignedInt(uint32_t value);
		virtual bool SetFloat(float value);
		virtual bool SetString(const char* value);
		virtual bool SetRGB(uint8_t r, uint8_t g, uint8_t b);
		virtual bool SetRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		virtual bool SetDescription(const char* value);
		inline bool SetString(const std::string& value) { return SetString(value.c_str()); }
		inline bool SetDescription(const std::string& value) { return SetDescription(value.c_str()); }

		[[nodiscard]] inline bool CheckValidValueType(Types t) const noexcept { return GetValueType() == t; }
		[[nodiscard]] inline bool GetBool() const noexcept { return _Value.b; }
		[[nodiscard]] inline char GetChar() const noexcept { return _Value.c; }
		[[nodiscard]] inline uint8_t GetUnsignedChar() const noexcept { return _Value.h; }
		[[nodiscard]] inline int32_t GetSignedInt() const noexcept { return _Value.i; }
		[[nodiscard]] inline uint32_t GetUnsignedInt() const noexcept { return _Value.u; }
		[[nodiscard]] inline float GetFloat() const noexcept { return _Value.f; }
		[[nodiscard]] inline const char* GetString() const noexcept { return _Value.s; }
		[[nodiscard]] inline Value::Color GetRGB() const noexcept { return _Value.rgba; }
		[[nodiscard]] inline Value::Color GetRGBA() const noexcept { return _Value.rgba; }

		[[nodiscard]] virtual string GetSection() const noexcept;
		[[nodiscard]] virtual string GetOptionName() const noexcept;
		[[nodiscard]] virtual string GetValueAsString() const noexcept;
		[[nodiscard]] virtual string GetDescription() const noexcept;

		XCPropertyReadOnly(GetValueType) Types ValueType;
		XCPropertyReadOnly(GetSection) string Section;
		XCPropertyReadOnly(GetOptionName) string OptionName;
		XCPropertyReadOnly(GetValueAsString) string ValueAsString;
		XCPropertyReadOnly(GetDescription) string Description;
	private:
		Value _Value;
		string _Description;
	};

	class CollectionSettings
	{
	protected:
		ICriticalSection _lock;
		std::map<UInt32, std::shared_ptr<Setting>> _collection;
	public:
		CollectionSettings() = default;
		virtual ~CollectionSettings() = default;

		virtual bool Add(const std::shared_ptr<Setting>& Setting);
		virtual bool Remove(const std::shared_ptr<Setting>& Setting);
		virtual bool RemoveByName(const char* SettingName);
		[[nodiscard]] virtual std::shared_ptr<Setting> FindByName(const char* SettingName) const noexcept;
		[[nodiscard]] virtual inline std::shared_ptr<Setting> FindByName(const string& SettingName) const noexcept { return FindByName(SettingName.c_str()); }

		[[nodiscard]] virtual std::shared_ptr<Setting> At(size_t Index) const noexcept;
		[[nodiscard]] virtual size_t Count() const noexcept;

		[[nodiscard]] std::shared_ptr<Setting> operator[](size_t Index) const noexcept { return At(Index); }

		virtual bool LoadFromFile(const char* FileName) = 0;
		virtual bool LoadFromFileRelative(long CategoryID, const char* FileName);

		CollectionSettings(const CollectionSettings&) = delete;
		CollectionSettings& operator=(const CollectionSettings&) = delete;
	};

	class TOMLCollectionSettings : public CollectionSettings
	{
	public:
		TOMLCollectionSettings() = default;
		virtual ~TOMLCollectionSettings() = default;

		virtual bool LoadFromFile(const char* FileName);	

		TOMLCollectionSettings(const TOMLCollectionSettings&) = delete;
		TOMLCollectionSettings& operator=(const TOMLCollectionSettings&) = delete;
	};
}