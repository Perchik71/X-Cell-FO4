// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleProfile.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellParseINI.h"
#include "XCellStringUtils.h"

#include <memory>

#include <f4se/GameSettings.h>

namespace XCell
{
	std::map<UInt64, shared_ptr<ParseINI>> _cache_inifiles;

	static bool __stdcall GetINIFromCache(ParseINI** Data, LPCSTR FileName)
	{
		if (!FileName)
			return false;

		auto Hash64 = Object(FileName).NameHash64;

		auto It = _cache_inifiles.find(Hash64);
		if (It != _cache_inifiles.end())
		{
			*Data = It->second.get();
			return true;
		}

		return false;
	}

	static bool __stdcall WriteINIToFile(ParseINI* Data, LPCSTR FileName)
	{
		if (!FileName || !Data)
			return false;

		WriteINI Writer;
		return Writer.Write(FileName, *Data);
	
		return false;
	}

	static bool __stdcall ParseINIAndStoreCache(ParseINI** Data, LPCSTR FileName)
	{
		if (!FileName)
			return false;

		auto Hash64 = Object(FileName).NameHash64;

		auto It = _cache_inifiles.find(Hash64);
		if (It != _cache_inifiles.end())
		{
			*Data = It->second.get();
			return true;
		}

		string FName = PathIsRelativeA(FileName) ?
			(Utils::GetApplicationPath() + FileName) : 
			FileName;

		auto Attr = GetFileAttributesA(FName.c_str());
		if ((Attr == INVALID_FILE_ATTRIBUTES) || (Attr == FILE_ATTRIBUTE_DIRECTORY))
			return false;

		auto Pair = make_pair(Hash64, make_shared<ParseINI>());
		if (!Pair.second->Parse(FName.c_str()))
			return false;

		_cache_inifiles.insert(Pair);
		*Data = Pair.second.get();

		return true;
	}

	static HRESULT __stdcall StringCopyBuffer(LPSTR DestString, DWORD DestSize, LPCSTR SourceString, DWORD SourceSize)
	{
		HRESULT hr = S_OK;

		if (DestSize < 2)
		{
			ZeroMemory(DestString, DestSize);
			return hr;
		}

		DWORD CopyBytes = SourceSize;
		if ((INT64)DestSize <= ((INT64)SourceSize - 2))
		{
			CopyBytes = DestSize - 2;
			hr = ERROR_INSUFFICIENT_BUFFER;
		}

		ZeroMemory(DestString, DestSize);
		memcpy(DestString, SourceString, CopyBytes);

		return hr;
	}

	static DWORD WINAPI HKGetPrivateProfileStringA(LPCSTR AppName, LPCSTR KeyName, LPCSTR DefaultValue,
		LPSTR ReturnedString, DWORD Size, LPCSTR FileName)
	{
		if (!FileName)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
			return 0;
		}
		if (!ReturnedString || Size < 2)
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return 0;
		}
		
		SetLastError(0);

		if (!DefaultValue)
			DefaultValue = "";

		//_MESSAGE("AppName: \"%s\", KeyName: \"%s\", DefaultValue: \"%s\", Size: \"%u\", FileName: \"%s\"", 
		//	AppName, KeyName, DefaultValue, Size, FileName);
		
		ParseINI* Data;
		// Enum all sections or error parse file
		if (!AppName || !ParseINIAndStoreCache(&Data, FileName))
			// There is no need to try to optimize or try parse itself
			return GetPrivateProfileStringA(AppName, KeyName, DefaultValue, ReturnedString, Size, FileName);

		DWORD Ret = 0, NeedSize = 0;
		HRESULT hr = S_OK;

		// Enum all keys in the section
		if (!KeyName)
		{
			if (!Data->Contains(AppName))
			{
			ReturnedDefaultString:
				Ret = NeedSize = strlen(DefaultValue);
				hr = StringCopyBuffer(ReturnedString, Size, DefaultValue, Ret);
				return Ret;
			}
			else
			{
				string s;

				auto& Section = Data->At(AppName);
				for (auto It = Section.begin(); It != Section.end(); It++)
				{
					s.append(It->second->Name);
					if (It != Section.end())
						s.push_back('\0');
				}

				NeedSize = s.size();
				hr = StringCopyBuffer(ReturnedString, Size, s.c_str(), s.size() - 1);
				Ret = s.size() - 1;
			}

			if (hr == ERROR_INSUFFICIENT_BUFFER)
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				_WARNING("GetPrivateProfileStringA: \"%s\":\"%s\" Buffer overflow. Returned size sets %u need %u", KeyName, AppName, 
					Size, NeedSize);

				Ret = Size - 1;
			}

			return Ret;
		}

		auto& Section = Data->At(AppName);

		if (!Section.Contains(KeyName))
			goto ReturnedDefaultString;
		else
		{
			auto s = Section[KeyName].AsStringWithoutQuote();
			Ret = s.length();
			hr = StringCopyBuffer(ReturnedString, Size, s.c_str(), Ret);

			if (hr == ERROR_INSUFFICIENT_BUFFER)
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				_WARNING("GetPrivateProfileStringA: \"%s\":\"%s\" Buffer overflow. Returned size sets %u need %u", KeyName, AppName,
					Size, Ret);

				Ret = Size - 1;
			}
		}

		/*_MESSAGE("ReturnedString: %u\"%s\", DefaultValue: \"%s\", AppName: \"%s\", KeyName: \"%s\"",
			Ret, ReturnedString, DefaultValue, AppName, KeyName);*/

		return Ret;
	}

	static UINT WINAPI HKGetPrivateProfileIntA(LPCSTR AppName, LPCSTR KeyName, INT DefaultValue, LPCSTR FileName)
	{
		if (!FileName)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
			return (UINT)DefaultValue;
		}
		if (!AppName || !KeyName)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return (UINT)DefaultValue;
		}

		SetLastError(0);

		//_MESSAGE("AppName: \"%s\", KeyName: \"%s\", DefaultValue: \"%i\", FileName: \"%s\"",
		//	AppName, KeyName, DefaultValue, FileName);

		ParseINI* Data;
		if (!ParseINIAndStoreCache(&Data, FileName))
			return GetPrivateProfileIntA(AppName, KeyName, DefaultValue, FileName);

		if (!Data->Contains(AppName))
			return (UINT)DefaultValue;
		
		auto& Section = Data->At(AppName);

		if (!Section.Contains(KeyName))
			return (UINT)DefaultValue;

		return (UINT)(Section[KeyName].AsInteger());
	}

	static BOOL WINAPI HKWritePrivateProfileStringA(LPCSTR AppName, LPCSTR KeyName, LPCSTR String, LPCSTR FileName)
	{
		if (!FileName)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
			return FALSE;
		}
		if (!AppName)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}

		SetLastError(0);

		ParseINI* Data;
		if (!GetINIFromCache(&Data, FileName))
			return WritePrivateProfileStringA(AppName, KeyName, String, FileName);

		if (!KeyName)
		{
			// The name of the key to be associated with a string.
			// If the key does not exist in the specified section, it is created.
			// If this parameter is NULL, the entire section, including all entries within the section, is deleted.

			if (!Data->Contains(AppName))
				return FALSE;

			Data->At(AppName).Clear();
			WriteINIToFile(Data, FileName);
			//Data->SetChanged(true);

			return TRUE;
		}

		if (!String)
		{
			// A null - terminated string to be written to the file.
			// If this parameter is NULL, the key pointed to by the key_name parameter is deleted.

			if (!Data->Contains(AppName))
				return FALSE;

			Data->At(AppName).Remove(KeyName);
			WriteINIToFile(Data, FileName);
			//Data->SetChanged(true);

			return TRUE;
		}

		Data->At(AppName)[KeyName].SetString(String);
		WriteINIToFile(Data, FileName);
		//Data->SetChanged(true);

		return TRUE;
	}

	static bool hk_subC30008()
	{
		auto iniDef = *(g_iniSettings.GetPtr());
		auto iniPref = *(g_iniPrefSettings.GetPtr());

		auto pSettingSrc = iniPref->data;

		do
		{
			auto pSettingDst = iniDef->Get(pSettingSrc->data->name);
			if (!pSettingDst)
			{
				auto pNewNode = new SettingCollectionList::Node;
				pNewNode->next = iniDef->data;
				pNewNode->data = pSettingSrc->data;
				iniDef->data = pNewNode;
			}
			
		} while (pSettingSrc = pSettingSrc->next);

		return false;
	}

	ModuleProfile::ModuleProfile(void* Context) :
		Module(Context, SourceName, CVarProfile)
	{}

	HRESULT ModuleProfile::InstallImpl()
	{
		if (GetModuleHandleA("PrivateProfileRedirector.dll"))
		{
			_WARNING("Mod \"PrivateProfileRedirector F4\" has been detected. X-Cell "
				"patch \"Profile\" does the same thing and will not be enabled.");
			return S_FALSE;
		}

		_cache_inifiles.clear();
		
		auto gContext = (XCell::Context*)Context;
		auto base = gContext->ProcessBase;

		//
		// profile optimizations:
		//
		// - Replacing functions WritePrivateProfileStringA, GetPrivateProfileStringA, GetPrivateProfileIntA
		//   They are outdated and constantly open and parsing the ini file. Complements Buffout 4, Buffout 4 NG.
		//   Incompatible with the mod https://www.nexusmods.com/fallout4/mods/33947 PrivateProfileRedirector.
		//   If that mod is installed, it needs to be disabled.

		REL::Impl::DetourIAT(base, "kernel32.dll", "WritePrivateProfileStringA", (uintptr_t)&HKWritePrivateProfileStringA);
		REL::Impl::DetourIAT(base, "kernel32.dll", "GetPrivateProfileStringA", (uintptr_t)&HKGetPrivateProfileStringA);
		REL::Impl::DetourIAT(base, "kernel32.dll", "GetPrivateProfileIntA", (uintptr_t)&HKGetPrivateProfileIntA);

		// Add new settings for plugins .ini
		REL::Impl::DetourCall(REL::ID(300), (UInt64)&hk_subC30008);
		
		return S_OK;
	}

	HRESULT ModuleProfile::ShutdownImpl()
	{
		// No recommended

		return S_FALSE;
	}
}