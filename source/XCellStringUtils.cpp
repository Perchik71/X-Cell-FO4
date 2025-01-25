// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <memory>

namespace XCell
{
	namespace Utils
	{
		wstring __stdcall AnsiToWide(const string& String)
		{
			auto l = MultiByteToWideChar(CP_ACP, 0, String.c_str(), (int32_t)String.length(), nullptr, 0);
			if (l <= 0) return L"";
			wstring temp;
			temp.resize(l);
			MultiByteToWideChar(CP_ACP, 0, String.c_str(), (int32_t)String.length(), temp.data(), (int32_t)temp.length());
			return temp;
		}

		string __stdcall WideToAnsi(const wstring& String)
		{
			auto l = WideCharToMultiByte(CP_ACP, 0, String.c_str(), (int32_t)String.length(), nullptr, 0, nullptr, nullptr);
			if (l <= 0) return "";
			string temp;
			temp.resize(l);
			WideCharToMultiByte(CP_ACP, 0, String.c_str(), (int32_t)String.length(), temp.data(), (int32_t)temp.length(), nullptr, nullptr);
			return temp;
		}

		string __stdcall AnsiToUtf8(const string& String)
		{
			auto Source = AnsiToWide(String);
			auto l = WideCharToMultiByte(CP_UTF8, 0, Source.c_str(), (int32_t)String.length(), nullptr, 0, nullptr, nullptr);
			if (l <= 0) return "";
			string temp;
			temp.resize(l);
			WideCharToMultiByte(CP_ACP, 0, Source.c_str(), (int32_t)Source.length(), temp.data(), (int32_t)temp.length(), nullptr, nullptr);
			return temp;
		}

		string __stdcall Utf8ToAnsi(const string& String)
		{
			auto l = MultiByteToWideChar(CP_UTF8, 0, String.c_str(), (int32_t)String.length(), nullptr, 0);
			if (l <= 0) return "";
			wstring temp;
			temp.resize(l);
			MultiByteToWideChar(CP_UTF8, 0, String.c_str(), (int32_t)String.length(), temp.data(), (int32_t)temp.length());
			return WideToAnsi(temp);
		}

		string& __stdcall Trim(string& String)
		{
			constexpr static char whitespaceDelimiters[] = " \t\n\r\f\v";

			if (!String.empty())
			{
				String.erase(String.find_last_not_of(whitespaceDelimiters) + 1);
				String.erase(0, String.find_first_not_of(whitespaceDelimiters));
			}

			return String;
		}

		string __stdcall GetApplicationPath()
		{
			string _app_path;
			const uint32_t BufferSize = 1024;
			
			auto Buffer = make_unique<char[]>((size_t)BufferSize + 1);
			if (Buffer && GetModuleFileNameA(GetModuleHandleA(nullptr), Buffer.get(), BufferSize))
			{
				PathRemoveFileSpecA(Buffer.get());
				_app_path = Buffer.get();
				_app_path += "\\";
			}

			return _app_path;
		}

		string __stdcall GetGameDataPath()
		{
			return GetApplicationPath() + "Data\\";
		}

		UInt32 __stdcall MurmurHash32A(const void* Key, size_t Len, UInt32 Seed)
		{
			/* -----------------------------------------------------------------------------
			// https://github.com/abrandoned/murmur2/blob/master/MurmurHash2.c#L65
			// MurmurHash2, by Austin Appleby
			//
			// Note - This code makes a few assumptions about how your machine behaves -
			// 1. We can read a 4-byte value from any address without crashing
			// 2. sizeof(int) == 4
			//
			// And it has a few limitations -
			//
			// 1. It will not work incrementally.
			// 2. It will not produce the same results on little-endian and big-endian
			//    machines.    */

			/* 'm' and 'r' are mixing constants generated offline.
			 They're not really 'magic', they just happen to work well.  */

			const uint32_t m = 0x5bd1e995;
			const int r = 24;

			/* Initialize the hash to a 'random' value */

			uint32_t h = (uint32_t)((size_t)Seed ^ Len);

			/* Mix 4 bytes at a time into the hash */

			const unsigned char* data = (const unsigned char*)Key;

			while (Len >= 4)
			{
				uint32_t k = *(uint32_t*)data;

				k *= m;
				k ^= k >> r;
				k *= m;

				h *= m;
				h ^= k;

				data += 4;
				Len -= 4;
			}

			/* Handle the last few bytes of the input array  */

			switch (Len)
			{
			case 3: h ^= data[2] << 16;
			case 2: h ^= data[1] << 8;
			case 1: h ^= data[0];
				h *= m;
			};

			/* Do a few final mixes of the hash to ensure the last few
			// bytes are well-incorporated.  */

			h ^= h >> 13;
			h *= m;
			h ^= h >> 15;

			return h;
		}

		UInt64 __stdcall MurmurHash64A(const void* Key, size_t Len, UInt64 Seed)
		{
			/*-----------------------------------------------------------------------------
			// https://github.com/abrandoned/murmur2/blob/master/MurmurHash2.c#L65
			// MurmurHash2, 64-bit versions, by Austin Appleby
			//
			// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment
			// and endian-ness issues if used across multiple platforms.
			//
			// 64-bit hash for 64-bit platforms
			*/
			const UInt64 m = 0xc6a4a7935bd1e995ull;
			const int r = 47;

			UInt64 h = Seed ^ (Len * m);

			const UInt64* data = (const UInt64*)Key;
			const UInt64* end = data + (Len / 8);

			while (data != end)
			{
				UInt64 k = *data++;

				k *= m;
				k ^= k >> r;
				k *= m;

				h ^= k;
				h *= m;
			}

			const unsigned char* data2 = (const unsigned char*)data;

			switch (Len & 7)
			{
			case 7: h ^= ((UInt64)data2[6]) << 48;
			case 6: h ^= ((UInt64)data2[5]) << 40;
			case 5: h ^= ((UInt64)data2[4]) << 32;
			case 4: h ^= ((UInt64)data2[3]) << 24;
			case 3: h ^= ((UInt64)data2[2]) << 16;
			case 2: h ^= ((UInt64)data2[1]) << 8;
			case 1: h ^= ((UInt64)data2[0]);
				h *= m;
			}

			h ^= h >> r;
			h *= m;
			h ^= h >> r;

			return h;
		}
	}
}