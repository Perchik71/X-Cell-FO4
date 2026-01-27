// Copyright © 2026 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

// Thanks WirelessLan for idea: https://github.com/WirelessLan/BSALimitExpander

#include "XCellModuleArchiveLimits.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellStringUtils.h"

#include <memory>
#include <unordered_map>

#include <xbyak/xbyak.h>
#include <f4se/GameTypes.h>
#include <f4se/GameEvents.h>
#include <f4se/NiTextures.h>

namespace XCell
{
	template<class Mutex>
	struct BSAutoLockDefaultPolicy
	{
	public:
		static void Lock(Mutex& Mutex) { Mutex.Lock(); }
		static void Unlock(Mutex& Mutex) { Mutex.Release(); }
	};

	extern template struct BSAutoLockDefaultPolicy<SimpleLock>;

	template<class Mutex>
	struct BSAutoLockReadLockPolicy
	{
	public:
		static void Lock(Mutex& Mutex) { Mutex.LockForRead(); }
		static void Unlock(Mutex& Mutex) { Mutex.UnlockRead(); }
	};

	extern template struct BSAutoLockReadLockPolicy<BSReadWriteLock>;

	template<class Mutex>
	struct BSAutoLockWriteLockPolicy
	{
	public:
		static void Lock(Mutex& Mutex) { Mutex.LockForWrite(); }
		static void Unlock(Mutex& Mutex) { Mutex.UnlockWrite(); }
	};

	extern template struct BSAutoLockWriteLockPolicy<BSReadWriteLock>;

	template<class Mutex, template <class> class Policy = BSAutoLockDefaultPolicy>
	class BSAutoLock
	{
	public:
		using mutex_type = Mutex;
		using policy_type = Policy<mutex_type>;
	private:
		mutex_type* _lock{ nullptr };
	public:
		explicit BSAutoLock(mutex_type& Mutex) : _lock(std::addressof(Mutex)) { policy_type::Lock(*_lock); }
		explicit BSAutoLock(mutex_type* Mutex) : _lock(Mutex) { if (_lock) policy_type::Lock(*_lock); }
		~BSAutoLock() { if (_lock) policy_type::Unlock(*_lock); }
	};

	template<class Mutex>
	BSAutoLock(Mutex&) -> BSAutoLock<Mutex>;

	template<class Mutex>
	BSAutoLock(Mutex*) -> BSAutoLock<Mutex>;

	extern template class BSAutoLock<SimpleLock, BSAutoLockDefaultPolicy>;
	extern template class BSAutoLock<BSReadWriteLock, BSAutoLockReadLockPolicy>;
	extern template class BSAutoLock<BSReadWriteLock, BSAutoLockWriteLockPolicy>;

	using BSAutoReadLock = BSAutoLock<BSReadWriteLock, BSAutoLockReadLockPolicy>;
	using BSAutoWriteLock = BSAutoLock<BSReadWriteLock, BSAutoLockWriteLockPolicy>;

	template <class Traits, class Accessor>
	struct BSTSmallIndexScatterTable
	{
		struct entry_type
		{
			std::uint16_t next = 0xFFFF;	// the next slot the check for conflict resolution, or 0xFFFF if no chain
			std::uint16_t index = 0xFFFF;	// the actual index, or 0xFFFF if invalid
		};

		std::uint64_t pad;
		entry_type* table;
		std::uint32_t size;
		std::uint32_t mask;
		std::uint32_t avail;
		std::uint32_t lastFree;
	};

	namespace BSGraphics
	{
		struct Texture
		{
			struct Data
			{
				std::uint32_t refCount;			// 00
				std::uint8_t dataFileIndex;		// 04
				std::uint8_t chunkCount;		// 05
				std::uint8_t chunkOffsetOrType;	// 06
				std::uint8_t dataFileHighIndex;	// 07
			};

			std::uint64_t unk00;
			std::uint64_t unk08;
			std::uint64_t unk10;
			Data* data;
		};
	}

	namespace BSResource
	{
		class ID
		{
			std::uint32_t file{ 0 };
			std::uint32_t ext{ 0 };
			std::uint32_t dir{ 0 };
		public:
			using GenerateIDFunctType = ID*(*)(ID*, const char*);
			inline static GenerateIDFunctType GenerateID_Orig{ nullptr };

			struct Hash 
			{
				[[nodiscard]] std::size_t operator()(const ID& id) const noexcept(true)
				{
					return 
						((std::hash<std::uint32_t>()(id.file) ^
						 (std::hash<std::uint32_t>()(id.ext) << 1)) >> 1) ^
						 (std::hash<std::uint32_t>()(id.dir) << 1);
				}
			};

			ID() noexcept(true) = default;
			ID(const char* path) noexcept(true) { Regen(path); }
			ID(const std::uint32_t& file, const std::uint32_t& ext, const std::uint32_t& dir)
				: file(file), ext(ext), dir(dir) {}
			ID(const ID& rhs) noexcept(true) { *this = rhs; }
			ID& operator=(const ID& rhs) noexcept(true) { file = rhs.file; ext = rhs.ext; dir = rhs.dir; return *this; }
			~ID() noexcept(true) = default;

			[[nodiscard]] inline bool operator==(const ID& rhs) const noexcept(true)
			{
				return file == rhs.file && ext == rhs.ext && dir == rhs.dir;
			}

			[[nodiscard]] inline bool operator!=(const ID& rhs) const noexcept(true)
			{
				return file != rhs.file || ext != rhs.ext || dir != rhs.dir;
			}

			[[nodiscard]] inline std::uint32_t GetNameHash() const noexcept(true) { return file; }
			[[nodiscard]] inline std::uint32_t GetExtHash() const noexcept(true) { return ext; }
			[[nodiscard]] inline std::uint32_t GetPathHash() const noexcept(true) { return dir; }

			inline void SetNameHash(std::uint32_t nValue) noexcept(true) { file = nValue; }
			inline void SetExtHash(std::uint32_t nValue) noexcept(true) { ext = nValue; }
			inline void SetPathHash(std::uint32_t nValue) noexcept(true) { dir = nValue; }
			inline void Regen(const char* path) noexcept(true) { BSResource::ID::GenerateID_Orig(this, path); }
		};

		using Storage = std::unordered_map<ID, std::uint16_t, ID::Hash>;

		enum class StorageType : std::uint8_t
		{
			kGeneral = 0,
			kTextures,
			kTotal
		};

		Storage Storages[static_cast<std::uint8_t>(StorageType::kTotal)];
		BSReadWriteLock StorageLocks[static_cast<std::uint8_t>(StorageType::kTotal)];

		static void PushArchiveIndex(const ID& id, std::uint32_t archIdx, StorageType archType) noexcept(true)
		{
			BSAutoWriteLock lock(StorageLocks[static_cast<std::uint8_t>(archType)]);
			Storages[static_cast<std::uint8_t>(archType)][id] = static_cast<std::uint16_t>(archIdx);
		}

		inline static void PushGeneralArchiveIndex(const ID& id, std::uint32_t archIdx) noexcept(true)
		{
			PushArchiveIndex(id, archIdx, StorageType::kGeneral);
		}

		inline static void PushTexturesArchiveIndex(const ID& id, std::uint32_t archIdx) noexcept(true)
		{
			PushArchiveIndex(id, archIdx, StorageType::kTextures);
		}

		static std::uint16_t FindArchiveIndex(const ID& id, StorageType archType) noexcept(true)
		{
			BSAutoWriteLock lock(StorageLocks[static_cast<std::uint8_t>(archType)]);

			auto it = Storages[static_cast<std::uint8_t>(archType)].find(id);
			return (it == Storages[static_cast<std::uint8_t>(archType)].end()) ?
				static_cast<std::uint16_t>(-1) : it->second;
		}

		inline static std::uint16_t FindGeneralArchiveIndex(const ID& id) noexcept(true)
		{
			return FindArchiveIndex(id, StorageType::kGeneral);
		}

		inline static std::uint16_t FindTexturesArchiveIndex(const ID& id) noexcept(true)
		{
			return FindArchiveIndex(id, StorageType::kTextures);
		}

		class StreamBase
		{
		public:
			StreamBase() = default;
			virtual ~StreamBase() = default;
		};

		class Stream : public StreamBase
		{};

		class AsyncStream : public StreamBase
		{};

		struct RegisteredEvent
		{};

		struct ClearRegistryEvent
		{};

		namespace Archive2
		{
			template<std::size_t total = std::numeric_limits<std::uint8_t>::max() + 1>
			struct Index :
				public BSTEventSink<RegisteredEvent>,
				public BSTEventSink<ClearRegistryEvent>
			{
				constexpr static auto MAX = total;

				struct NameIDAccess;

				struct EntryHeader
				{
					ID nameID;
					std::uint8_t dataFileIndex{ 0 };
					std::uint8_t chunkCount{ 0 };
					std::uint16_t chunkOffsetOrType{ 0 };

					[[nodiscard]] bool IsChunk() const noexcept { return this->chunkOffsetOrType != 0; }
					[[nodiscard]] bool IsLoose() const noexcept { return this->chunkOffsetOrType == 0; }
				};
				static_assert(sizeof(EntryHeader) == 0x10);

				BSTSmallIndexScatterTable<ID, NameIDAccess> nameTable;
				BSTSmartPointer<Stream> dataFiles[MAX];
				BSTSmartPointer<AsyncStream> asyncDataFiles[MAX];
				ID dataFileNameIDs[MAX];
				std::uint32_t dataFileCount;
				char unk[0x20];
				BSReadWriteLock lock;
			};

			Index<std::numeric_limits<std::uint16_t>::max() + 1>* NewManager{ nullptr };

			using Stream__Assign = void (*)(BSTSmartPointer<Stream>&, BSTSmartPointer<Stream>&);
			inline static Stream__Assign Stream__Assign_Orig{ nullptr };

			using BSTSmallIndexScatterTableUtil__NewTable = BSTSmallIndexScatterTable<ID, Index<>::NameIDAccess>::entry_type* (*)(std::uint32_t);
			inline static BSTSmallIndexScatterTableUtil__NewTable BSTSmallIndexScatterTableUtil__NewTable_Orig{ nullptr };

			using BSTSmallIndexScatterTableTraits__Insert = bool (*)(BSTSmallIndexScatterTable<ID, Index<>::NameIDAccess>&, std::uint32_t, ID*&);
			inline static BSTSmallIndexScatterTableTraits__Insert BSTSmallIndexScatterTableTraits__Insert_Orig{ nullptr };

			using BSTSmallIndexScatterTableTraits__Resize = void (*)(BSTSmallIndexScatterTable<ID, Index<>::NameIDAccess>&, ID*&);
			inline static BSTSmallIndexScatterTableTraits__Resize BSTSmallIndexScatterTableTraits__Resize_Orig{ nullptr };

			inline static BSTSmallIndexScatterTable<ID, Index<>::NameIDAccess>::entry_type* EndTable{ nullptr };

			static void AddDataFile(Index<>& self, BSTSmartPointer<Stream>& stream, ID& id, std::uint32_t index)
			{
				Stream__Assign_Orig(NewManager->dataFiles[index], stream);
				XCThisVirtualCall<std::uint32_t>(0x80, stream.get(), &NewManager->asyncDataFiles[index]);

				if (self.dataFileCount != index)
					return;

				//_MESSAGE("DEBUG: %d %d", self.dataFileCount, index);

				NewManager->dataFileNameIDs[index] = id;
				auto* p_id = NewManager->dataFileNameIDs;

				if (self.nameTable.table == EndTable)
				{
					// constant initialization check that XCell/CKPE is cut out
					constexpr static auto MEMORY_INITIAZE_FLAG = 2;

					self.nameTable.avail = MEMORY_INITIAZE_FLAG;
					self.nameTable.table = BSTSmallIndexScatterTableUtil__NewTable_Orig(MEMORY_INITIAZE_FLAG);
				}
				else
				{
					if (!BSTSmallIndexScatterTableTraits__Insert_Orig(self.nameTable, index, p_id))
						BSTSmallIndexScatterTableTraits__Resize_Orig(self.nameTable, p_id);
					else goto __ll_end;
				}

				BSTSmallIndexScatterTableTraits__Insert_Orig(self.nameTable, index, p_id);
			__ll_end:
				self.dataFileCount++;
			}

			static void Hook_Init()
			{
				*(UInt64*)&Stream__Assign_Orig = REL::ID(342);
				*(UInt64*)&BSTSmallIndexScatterTableUtil__NewTable_Orig = REL::ID(343);
				*(UInt64*)&BSTSmallIndexScatterTableTraits__Insert_Orig = REL::ID(344);
				*(UInt64*)&BSTSmallIndexScatterTableTraits__Resize_Orig = REL::ID(345);
				*(UInt64*)&EndTable = REL::ID(346);

				REL::Impl::DetourJump(REL::ID(341), (UInt64)&AddDataFile);

				struct AddDataFromReaderPatch_AE : Xbyak::CodeGenerator
				{
					AddDataFromReaderPatch_AE(std::uintptr_t targetAddr, std::uintptr_t funcAddr)
					{
						// run erase code
						mov(ptr[rsp + 0x3C], r12b);

						push(rax);
						push(rcx);
						push(rdx);
						sub(rsp, 0x28);

						// get ID
						lea(rcx, ptr[rsp + 0x70]);
						// get index arch
						mov(edx, ptr[rbp + 0xE8]);
						// call link ID with arch
						mov(rax, funcAddr);
						call(rax);

						add(rsp, 0x28);
						pop(rdx);
						pop(rcx);
						pop(rax);

						// return back (ret)
						jmp(ptr[rip]);
						dq(targetAddr + 5);
					}
				};

				auto target = REL::ID(347);
				auto patch = new AddDataFromReaderPatch_AE(target, (std::uintptr_t)&PushGeneralArchiveIndex);
				REL::Impl::DetourJump(target, (std::uintptr_t)patch->getCode());
			}
		}

		namespace SDirectory2
		{
			static void InsertReplicatedGeneralID(const ID& id, std::uint32_t repDir) noexcept(true)
			{
				std::uint16_t index = FindGeneralArchiveIndex(id);
				if (index == static_cast<std::uint16_t>(-1))
					return;

				ID repId = id;
				repId.SetPathHash(repDir);
				PushGeneralArchiveIndex(repId, index);
			}

			static void Hook_Init()
			{
				////////////////////////////////////////////////
				// DEFAULT
				////////////////////////////////////////////////
				{
					struct FindGeneralPatch_AE : Xbyak::CodeGenerator
					{
						FindGeneralPatch_AE(std::uintptr_t target, std::uintptr_t funcAddr)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rcx);
							sub(rsp, 0x28);
							lea(rcx, ptr[rbp + 0x148]);
							call(ptr[rip + funcLabel]);
							add(rsp, 0x28);
							pop(rcx);

							cmp(eax, 0xFFFF);
							jne("RET");
							movzx(eax, byte[rbp + 0x154]);

							L("RET");
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 7);

							L(funcLabel);
							dq(funcAddr);
						}
					};

					auto target = REL::ID(350);
					auto patch = new FindGeneralPatch_AE(target, (std::uintptr_t)&FindGeneralArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				{
					struct GetDataFilePatch_AE : Xbyak::CodeGenerator
					{
						GetDataFilePatch_AE(std::uintptr_t target)
						{
							mov(rcx, (std::uintptr_t)Archive2::NewManager->dataFiles);
							mov(rdx, ptr[rcx + rax * 8]);
							// return back (ret)
							jmp(ptr[rip]);
							dq(target + 5);
						}
					};

					auto target = REL::ID(351);
					auto patch = new GetDataFilePatch_AE(target);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// ASYNC
				////////////////////////////////////////////////
				{
					struct FindGeneralPatch_AE : Xbyak::CodeGenerator
					{
						FindGeneralPatch_AE(std::uintptr_t target, std::uintptr_t funcAddr)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rcx);
							sub(rsp, 0x28);

							lea(rcx, ptr[rdi + 0x148]);
							call(ptr[rip + funcLabel]);

							add(rsp, 0x28);
							pop(rcx);

							cmp(eax, 0xFFFF);
							jne("RET");
							movzx(eax, byte[rdi + 0x154]);

							L("RET");
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 7);

							L(funcLabel);
							dq(funcAddr);
						}
					};

					auto target = REL::ID(352);
					auto patch = new FindGeneralPatch_AE(target, (std::uintptr_t)&FindGeneralArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				{
					struct GetAsyncDataFilePatch_AE : Xbyak::CodeGenerator
					{
						GetAsyncDataFilePatch_AE(std::uintptr_t target)
						{
							mov(rcx, (std::uintptr_t)Archive2::NewManager->asyncDataFiles);
							mov(rdx, ptr[rcx + rax * 8]);
							// return back (ret)
							jmp(ptr[rip]);
							dq(target + 5);
						}
					};

					auto target = REL::ID(353);
					auto patch = new GetAsyncDataFilePatch_AE(target);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// Replicate Dir
				////////////////////////////////////////////////
				{
					struct ReplicateDirToPatch_AE : Xbyak::CodeGenerator
					{
						ReplicateDirToPatch_AE(std::uintptr_t targetAddr, std::uintptr_t funcAddr)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rax);
							push(rcx);
							push(rdx);
							push(r8);
							sub(rsp, 0x20);

							lea(rcx, ptr[rdi]);
							mov(edx, ebx);
							call(ptr[rip + funcLabel]);

							add(rsp, 0x20);
							pop(r8);
							pop(rdx);
							pop(rcx);
							pop(rax);

							mov(ptr[rdi + 0x8], ebx);
							mov(ptr[rdi], ecx);
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(targetAddr + 0x6);

							L(funcLabel);
							dq(funcAddr);
						}
					};

					auto target = REL::ID(354);
					auto patch = new ReplicateDirToPatch_AE(target, (std::uintptr_t)InsertReplicatedGeneralID);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
			}
		}
	}

	namespace BSScaleformImageLoader
	{
		static void Hook_Init()
		{
			struct BSScaleformImageLoader_AE : Xbyak::CodeGenerator
			{
				BSScaleformImageLoader_AE(std::uintptr_t target)
				{
					test(rcx, rcx);
					jne("JMP");
					xor_(al, al);
					ret();
					L("JMP");
					// return back (ret)
					jmp(ptr[rip]);
					dq(target);
				}
			};

			auto patch = new BSScaleformImageLoader_AE(REL::ID(356));
			REL::Impl::DetourJump(REL::ID(355), (std::uintptr_t)patch->getCode());
		}
	}

	namespace BSTextureIndex
	{
		BSResource::ID dataFileNameIDs[std::numeric_limits<std::uint16_t>::max() + 1];

		static void Hook_Init()
		{
			// movzx r15d, r13b -> mov r15d, r13d; nop;
			REL::Impl::Patch(REL::ID(360), { 0x45, 0x89, 0xEF, 0x90 });

			struct AddDataFilePatch_AE : Xbyak::CodeGenerator
			{
				AddDataFilePatch_AE(std::uintptr_t target)
				{
					// orig
					// mov eax, dword ptr ds : [rsi + 0x28]
					// lea rcx, qword ptr ds : [r15 + r15 * 2]
					// lea rdx, qword ptr ds : [rcx * 4]
					// mov dword ptr ds : [rdx + r13 + 0x98DA8] , eax
					// mov ecx, dword ptr ds : [rsi + 0x24]
					// mov eax, dword ptr ds : [rsi + 0x20]
					// shl rcx, 0x20
					// or rcx, rax
					// mov dword ptr ds : [rdx + r13 + 0x98DA0] , ecx
					// shr rcx, 0x20
					// mov dword ptr ds : [rdx + r13 + 0x98DA4] , ecx

					push(rbx);
					push(rdx);
					mov(rbx, (std::uintptr_t)dataFileNameIDs);
					mov(eax, ptr[rsi + 0x28]);
					lea(rdx, ptr[r15 + r15 * 2]);
					shl(rdx, 2);
					mov(ptr[rbx + rdx + 8], eax);
					mov(ecx, ptr[rsi + 0x24]);
					mov(eax, ptr[rsi + 0x20]);
					shl(rcx, 0x20);
					or_(rcx, rax);
					mov(ptr[rbx + rdx], ecx);
					shr(rcx, 0x20);
					mov(ptr[rbx + rdx + 4], ecx);
					pop(rdx);
					pop(rbx);

					// return back (ret)
					jmp(ptr[rip]);
					dq(target + 0x38);
				}
			};

			auto target = REL::ID(361);
			auto patch = new AddDataFilePatch_AE(target);
			REL::Impl::DetourJump(target, (UInt64)patch->getCode());
		}
	}

	namespace BSTextureStreamer
	{
		namespace Manager
		{
			struct TextureRequest 
			{
				BSResource::Archive2::Index<>::EntryHeader header;
				char unk10[0x68];
				BSFixedString unk78;
				char unk80[0x48];
				NiTexture* texture;
				BSFixedString texturePath;
				char unkD8[0x38];
			};
			static_assert(sizeof(TextureRequest) == 0x110);

			static void ProcessPath(const char* inputPath, char* outputPath) noexcept(true)
			{
				char temp[MAX_PATH]{};
				size_t i = 0;

				for (; inputPath[i] && (i < MAX_PATH - 1); i++) 
				{
					char c = inputPath[i];

					if ((c >= 'A') && (c <= 'Z'))
						c += 32;

					if (c == '/')
						c = '\\';

					temp[i] = c;
				}
				temp[i] = '\0';

				const char* p = temp;
				const char* dataPos = strstr(p, "data\\");

				if (dataPos)
					p = dataPos + 5;

				if (strncmp(p, "textures\\", 9) == 0)
				{
					strcpy_s(outputPath, MAX_PATH, p);
					return;
				}

				strcpy_s(outputPath, MAX_PATH, "textures\\");
				strcat_s(outputPath, MAX_PATH, p);
			}

			static std::uint16_t FindArchiveIndexByTextureRequest(const TextureRequest& request) noexcept(true)
			{
				auto fileName = request.texturePath.c_str();
				if (fileName && fileName[0])
				{
					char processedPath[MAX_PATH];
					ProcessPath(fileName, processedPath);

					BSResource::ID id(processedPath);
					return BSResource::FindTexturesArchiveIndex(id);
				}

				if (request.texture && request.texture->rendererData)
				{
					auto Renderer = (BSGraphics::Texture*)request.texture->rendererData;
					if (Renderer->data)
						return (Renderer->data->dataFileHighIndex << 8) | Renderer->data->dataFileIndex;
				}

				BSResource::ID id = request.header.nameID;
				return BSResource::FindTexturesArchiveIndex(id);
			}

			static void Hook_Init()
			{
				////////////////////////////////////////////////
				// Process Event
				////////////////////////////////////////////////
				{
					struct ProcessEventPatch_AE : Xbyak::CodeGenerator 
					{
						ProcessEventPatch_AE(std::uintptr_t target, std::uintptr_t func) 
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							mov(ptr[rsp + 0x3C], r13b);

							push(rcx);
							push(rdx);
							sub(rsp, 0x20);

							lea(rcx, ptr[rsp + 0x60]);
							mov(edx, r13d);

							call(ptr[rip + funcLabel]);

							add(rsp, 0x20);
							pop(rdx);
							pop(rcx);

							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 0x5);

							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(362);
					auto patch = new ProcessEventPatch_AE(target, (std::uintptr_t)BSResource::PushTexturesArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// Load chunks
				////////////////////////////////////////////////
				{
					struct LoadChunksPatch_AE : Xbyak::CodeGenerator
					{
						LoadChunksPatch_AE(std::uintptr_t target, std::uintptr_t func)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rax);
							push(rcx);
							push(rdx);
							push(r10);
							push(r11);
							sub(rsp, 0x28);

							lea(rcx, ptr[rdx]);
							call(ptr[rip + funcLabel]);
							mov(ebx, eax);

							add(rsp, 0x28);
							pop(r11);
							pop(r10);
							pop(rdx);
							pop(rcx);
							pop(rax);

							cmp(ebx, 0xFFFF);
							jne("RET");
							movzx(ebx, byte[rdx + 0xC]);

							L("RET");
							movzx(edi, byte[rdx + 0xD]);
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 8);

							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(363);
					auto patch = new LoadChunksPatch_AE(target, (std::uintptr_t)BSResource::FindTexturesArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// Start streaming chunks
				////////////////////////////////////////////////
				{
					struct StartStreamingChunksPatch_AE : Xbyak::CodeGenerator
					{
						StartStreamingChunksPatch_AE(std::uintptr_t target, std::uintptr_t func)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rcx);
							push(rdx);
							sub(rsp, 0x20);
							lea(rcx, ptr[r14]);
							call(ptr[rip + funcLabel]);
							mov(r8d, eax);
							add(rsp, 0x20);
							pop(rdx);
							pop(rcx);

							cmp(r8d, 0xFFFF);
							jne("RET");
							movzx(r8d, byte[r14 + 0xC]);

							L("RET");
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 5);

							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(364);
					auto patch = new StartStreamingChunksPatch_AE(target, (std::uintptr_t)FindArchiveIndexByTextureRequest);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// Decompress streamed load
				////////////////////////////////////////////////
				{
					struct DecompressStreamedLoadPatch_AE : Xbyak::CodeGenerator
					{
						DecompressStreamedLoadPatch_AE(std::uintptr_t target, std::uintptr_t func)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rax);
							push(rcx);
							push(rdx);
							sub(rsp, 0x28);
							lea(rcx, ptr[r13]);
							call(ptr[rip + funcLabel]);
							mov(r8d, eax);
							add(rsp, 0x28);
							pop(rdx);
							pop(rcx);
							pop(rax);

							cmp(r8d, 0xFFFF);
							jne("RET");
							movzx(r8d, byte[r13 + 0xC]);

							L("RET");
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 5);

							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(365);
					auto patch = new DecompressStreamedLoadPatch_AE(target, (std::uintptr_t)FindArchiveIndexByTextureRequest);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// BSGraphics::Renderer::CreateStreamingTexture
				////////////////////////////////////////////////
				{
					struct CreateStreamingTexturePatch_AE : Xbyak::CodeGenerator
					{
						CreateStreamingTexturePatch_AE(std::uintptr_t target, std::uintptr_t func)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rax);
							push(rcx);
							sub(rsp, 0x20);

							lea(rcx, ptr[rsi]);
							call(ptr[rip + funcLabel]);

							mov(edx, eax);

							add(rsp, 0x20);
							pop(rcx);
							pop(rax);

							cmp(edx, 0xFFFF);
							je("RET");

							mov(byte[rax + 4], dl);
							mov(byte[rax + 7], dh);

							L("RET");
							movzx(edx, byte[rcx + 0x3C]);
							mov(ptr[rax + 6], dl);
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 0xD);

							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(366);
					auto patch = new CreateStreamingTexturePatch_AE(target, (std::uintptr_t)BSResource::FindTexturesArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// BSGraphics::CreateStreamingDDSTexture
				////////////////////////////////////////////////
				{
					struct CreateStreamingDDSTexturePatch_AE : Xbyak::CodeGenerator
					{
						CreateStreamingDDSTexturePatch_AE(std::uintptr_t target, std::uintptr_t func)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							push(rcx);
							sub(rsp, 0x28);
							lea(rcx, ptr[rsi]);
							call(ptr[rip + funcLabel]);
							add(rsp, 0x28);
							pop(rcx);

							cmp(eax, 0xFFFF);
							jne("RET");
							movzx(eax, byte[rsi + 0xC]);

							L("RET");
							mov(ptr[r14 + 0x12], ax);
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(target + 5);
							
							L(funcLabel);
							dq(func);
						}
					};

					auto target = REL::ID(367);
					auto patch = new CreateStreamingDDSTexturePatch_AE(target, (std::uintptr_t)BSResource::FindTexturesArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
				////////////////////////////////////////////////
				// ThreadProc
				////////////////////////////////////////////////
				{
					struct ThreadProcPatch_AE : Xbyak::CodeGenerator
					{
						ThreadProcPatch_AE(std::uintptr_t a_target, std::uintptr_t a_funcAddr)
						{
							Xbyak::Label retnLabel;
							Xbyak::Label funcLabel;

							sub(rsp, 0x20);
							lea(rcx, ptr[r13]);
							call(ptr[rip + funcLabel]);
							mov(r8d, eax);
							add(rsp, 0x20);

							cmp(r8d, 0xFFFF);
							jne("RET");
							movzx(r8d, byte[r13 + 0xC]);

							L("RET");
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(a_target + 5);

							L(funcLabel);
							dq(a_funcAddr);
						}
					};

					auto target = REL::ID(368);
					auto patch = new ThreadProcPatch_AE(target, (std::uintptr_t)BSResource::FindTexturesArchiveIndex);
					REL::Impl::DetourJump(target, (UInt64)patch->getCode());
				}
			}
		}
	}

	ModuleArchiveLimits::ModuleArchiveLimits(void* Context) :
		Module(Context, SourceName, CVarArchiveLimits)
	{}

	HRESULT ModuleArchiveLimits::InstallImpl()
	{
		if (REL::Version() == RUNTIME_VERSION_1_11_191)
		{
			auto gContext = (XCell::Context*)Context;
			auto base = gContext->ProcessBase;

			*(std::uintptr_t*)&BSResource::ID::GenerateID_Orig = REL::ID(340);
			BSResource::Archive2::NewManager = new BSResource::Archive2::Index<std::numeric_limits<std::uint16_t>::max() + 1>;

			BSResource::Archive2::Hook_Init();
			BSResource::SDirectory2::Hook_Init();
			BSScaleformImageLoader::Hook_Init();
			BSTextureIndex::Hook_Init();
			BSTextureStreamer::Manager::Hook_Init();

			MessageBoxA(0, "Breakpoint message", "Debug", 0);
		}
		
		return S_OK;
	}

	HRESULT ModuleArchiveLimits::ShutdownImpl()
	{
		// No recommended

		return S_FALSE;
	}
}