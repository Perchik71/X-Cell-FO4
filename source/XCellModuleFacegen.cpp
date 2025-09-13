// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleFacegen.h"

// F4SE
#include <f4se/GameObjects.h>
#include <f4se/GameForms.h>
#include <f4se/GameData.h>
#include <f4se/GameAPI.h>

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellParseINI.h"
#include "XCellStringUtils.h"

namespace XCell
{
	enum TESActorFlags_XCELL
	{
		kFlagSimpleActor = 0x100000,
	};

	enum TESActorTemplateFlags_XCELL
	{
		kFlagTemplateTraits = 1 << 16,
	};

	static BGSKeyword** KeywordIsChildPlayer = nullptr;
	static vector<UInt32> FacegenPrimaryExceptionFormIDs =
	{
		// Since it has no protection in the game

		0x26F0A,	// MQ102PlayerSpouseCorpseMale
		0x26F36,	// MQ102PlayerSpouseCorpseFemale
		0xA7D34,	// MQ101PlayerSpouseMale
		0xA7D35,	// MQ101PlayerSpouseFemale
		0x246BF0,	// MQ101PlayerSpouseMale_NameOnly
		0x246BF1,	// MQ101PlayerSpouseFemale_NameOnly
	};
	static vector<UInt32> FacegenExceptionFormIDs;
	static DataHandler** FacegenDataHandler = nullptr;
	static ParseINI FacegenExceptionINI;

	namespace BSTextureDB
	{
#pragma pack(push, 1)
		struct EntryID
		{
			UInt32 Cache;
			char Type[4];
			UInt32 Unk;
		};
#pragma pack(pop)

		// Working buried function.
		static UInt64 FacegenPathPrintf = 0;
		static UInt64 CreateEntryID = 0;

		static bool __stdcall FormatPath__And__ExistIn(TESNPC* NPC, const char* DestPath,
			UInt32 Size, UInt32 TextureIndex) noexcept(true)
		{
			EntryID ID;
			XCFastCall<void>(FacegenPathPrintf, NPC, DestPath, Size, TextureIndex);
			return XCFastCall<bool>(CreateEntryID, DestPath + 14, &ID);
		}
	}

	static bool __stdcall GetLoadOrderByFormID(const string& PluginName, UInt32& FormID) noexcept(true)
	{
		constexpr static UInt16 INVALID_INDEX = (UInt16)-1;

		__try
		{
			UInt16 IndexPlugin = INVALID_INDEX;

			if (!PluginName.empty())
			{
				// Search among master, default plugins
				if ((IndexPlugin = (*FacegenDataHandler)->GetLoadedModIndex(PluginName.c_str())) != INVALID_INDEX)
					FormID = (FormID & (0x00FFFFFF)) | (IndexPlugin << 24);
				// Search among light master plugins
				else if ((IndexPlugin = (*FacegenDataHandler)->GetLoadedLightModIndex(PluginName.c_str())) != INVALID_INDEX)
					FormID = (FormID & (0x00000FFF)) | (IndexPlugin << 12) | 0xFE000000;
				// If there is no such thing, then it is a waste of a stupid user's time
				else
				{
					_ERROR("Failed NPC added (no found plugin) \"%s\" (%08X)", PluginName.c_str(), FormID);
					return false;
				}
			}

			return true;
		}
		__except (1)
		{
			_ERROR("Failed NPC added (fatal error) \"%s\" (%08X)", PluginName.c_str(), FormID);
			return false;
		}
	}

	static bool __stdcall HasOwnFace(TESNPC* NPC) noexcept(true)
	{
		if (!((NPC->actorData.unk10 & TESActorTemplateFlags_XCELL::kFlagTemplateTraits) == 
			TESActorTemplateFlags_XCELL::kFlagTemplateTraits))
			return true;
		return !NPC->templateNPC;
	}

	static bool __stdcall CanUsePreprocessingHead(TESNPC* NPC) noexcept(true)
	{
		if (!NPC) return false;
		// if template is specified, take face from template
		if (!HasOwnFace(NPC))
		{
			// list them until find the main form.
			while (!HasOwnFace(NPC))
				NPC = NPC->templateNPC;
		}
		// if the mod has set this option, i prohibit the use of preliminary data.
		if ((NPC->actorData.flags & TESActorBaseData::kFlagIsPreset) ||
			(NPC->actorData.flags & TESActorFlags_XCELL::kFlagSimpleActor))
			return false;
		// check if the NPC is a relative or a template for the player.
		auto size = NPC->keywords.numKeywords;
		for (size_t i = 0; i < size; i++)
			if (NPC->keywords.keywords[i] == *KeywordIsChildPlayer)
				return false;
		// optionally exclude some NPCs.
		for (auto it_except : FacegenExceptionFormIDs)
			if (NPC->formID == it_except)
				return false;
		// player form can't have a facegen.
		if (NPC->formID == 0x7)
			return false;
		// check exists diffuse texture.
		char buf[MAX_PATH];
		bool result = BSTextureDB::FormatPath__And__ExistIn(NPC, buf, MAX_PATH, 0);
		if (!result && CVarDbgFacegenOutput->GetBool())
		{
			auto fullName = NPC->GetFullName();
			if (!fullName) fullName = "<Unknown>";

			Console_Print("XCELL: NPC \"%s\" (%08X) don't have facegen", fullName, NPC->formID);
			_WARNING("NPC \"%s\" (%08X) don't have facegen", fullName, NPC->formID);
		}
		return result;
	}

	ModuleFacegen::ModuleFacegen(void* Context) :
		Module(Context, SourceName, CVarFacegen, XCELL_MODULE_QUERY_DATA_READY)
	{
		FacegenDataHandler = g_dataHandler.GetPtr();
		GameDataReadyLinker.OnListener = (EventGameDataReadySourceLink::EventFunctionType)(&ModuleFacegen::Listener);
	}

	HRESULT ModuleFacegen::Listener()
	{
		FacegenExceptionFormIDs = FacegenPrimaryExceptionFormIDs;
		if (FacegenExceptionINI.Contains("facegen_exception"))
		{
			auto& Section = FacegenExceptionINI.At("facegen_exception");
			for (auto& Option : Section)
			{
				auto Exception = Option.second->AsStringWithoutQuote();
				if (Exception.empty())
					continue;

				UInt32 FormID = 0;
				string Value, PluginName;

				auto It = Exception.find_first_of(':');
				if (It != std::string::npos)
				{
					PluginName = Exception.substr(It + 1);
					Value = Exception.substr(0, It);
					Utils::Trim(PluginName);
					Utils::Trim(Value);
				}
				else
					Value = Exception;

				if (Value.find_first_of("0x") == 0)
					FormID = strtoul(Value.c_str() + 2, nullptr, 16);
				else
					FormID = strtoul(Value.c_str(), nullptr, 10);

				if (GetLoadOrderByFormID(PluginName, FormID))
				{
					_MESSAGE("Skip NPC added \"%s\" (%08X)", Option.second->Name.c_str(), FormID);
					FacegenExceptionFormIDs.push_back(FormID);
				}
			}
		}

		return S_OK;
	}

	HRESULT ModuleFacegen::InstallImpl()
	{
		FacegenExceptionINI.Parse((Utils::GetGameDataPath() + "F4SE\\Plugins\\x-cell-exceptions.ini").c_str());

		// Working buried function.
		BSTextureDB::FacegenPathPrintf = REL::ID(200);	
		BSTextureDB::CreateEntryID = REL::ID(225);
		UInt64 Offset = REL::ID(205);

		// Remove useless stuff.
		if (REL::Version() == RUNTIME_VERSION_1_10_163)
		{
			REL::Impl::PatchNop(Offset, 0x1F);
			REL::Impl::PatchNop(Offset + 0x25, 0x37);

			// mov rcx, r13
			// lea rdx, qword ptr ss:[rbp-0x10]
			// mov r8d, 0x104
			// mov r9d, esi
			// cmp r9d, 0x2
			// mov eax, 0x7
			// cmove r9d, eax
			REL::Impl::Patch(Offset + 0x30, { 0x4C, 0x89, 0xE9, 0x48, 0x8D, 0x55, 0xF0, 0x41, 0xB8, 0x04, 0x01, 0x00, 0x00,
				0x41, 0x89, 0xF1, 0x41, 0x83, 0xF9, 0x02, 0xB8, 0x07, 0x00, 0x00, 0x00, 0x44, 0x0F, 0x44, 0xC8 });
			// call
			REL::Impl::DetourCall(Offset + 0x4D, BSTextureDB::FacegenPathPrintf);
		}
		else if (REL::Version() == RUNTIME_VERSION_1_10_984)
		{
			REL::Impl::PatchNop(Offset, 0x1C);
			REL::Impl::PatchNop(Offset + 0x23, 0x36);

			// mov rcx, r13
			// lea rdx, qword ptr ss:[rbp-0x40]
			// mov r8d, 0x104
			// mov r9d, edi
			// cmp r9d, 0x2
			// mov eax, 0x7
			// cmove r9d, eax
			REL::Impl::Patch(Offset + 0x2D, { 0x4C, 0x89, 0xE9, 0x48, 0x8D, 0x55, 0xC0, 0x41, 0xB8, 0x04, 0x01, 0x00, 0x00,
				0x41, 0x89, 0xF9, 0x41, 0x83, 0xF9, 0x02, 0xB8, 0x07, 0x00, 0x00, 0x00, 0x44, 0x0F, 0x44, 0xC8 });
			// call
			REL::Impl::DetourCall(Offset + 0x4A, BSTextureDB::FacegenPathPrintf);
		}
		else 
			return E_FAIL;

		// added for check player npc
		KeywordIsChildPlayer = (BGSKeyword**)(REL::ID(210));

#if 0
		REL::Impl::PatchNop(REL::ID(220), 0x27);
#else
		REL::Impl::DetourJump(REL::ID(215), (UInt64)&CanUsePreprocessingHead);
#endif

		return S_OK;
	}

	HRESULT ModuleFacegen::ShutdownImpl()
	{
		// Imposible

		return S_FALSE;
	}
}