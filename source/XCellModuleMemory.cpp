// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <Voltek.MemoryManager.h>

#include "XCellTableID.h"
#include "XCellModuleMemory.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

namespace XCell
{
	constexpr auto MEM_GB = 1073741824;

	class memory_manager
	{
	public:
		memory_manager()
		{
			voltek::scalable_memory_manager_initialize();
		}

		memory_manager(const memory_manager&) = default;
		memory_manager& operator=(const memory_manager&) = default;

		static void* alloc(size_t size, size_t alignment, bool aligned = false, bool zeroed = true)
		{
			if (!aligned)
				alignment = 4;

			if (!size)
				return voltek::scalable_alloc(0);

			// Reset last error
			SetLastError(0);

			XCAssertWithFormattedMessage((alignment != 0) && ((alignment % 2) == 0), "Alignment is fucked: %llu", alignment);

			if ((alignment & (alignment - 1)) != 0)
			{
				alignment--;
				alignment |= alignment >> 1;
				alignment |= alignment >> 2;
				alignment |= alignment >> 4;
				alignment |= alignment >> 8;
				alignment |= alignment >> 16;
				alignment++;
			}

			if ((size % alignment) != 0)
				size = ((size + alignment - 1) / alignment) * alignment;

			void* ptr = voltek::scalable_alloc(size);
			if (ptr && zeroed) memset(ptr, 0, size);


			if (!ptr)
			{
				MEMORYSTATUSEX statex = { 0 };
				statex.dwLength = sizeof(MEMORYSTATUSEX);
				if (!GlobalMemoryStatusEx(&statex))
					return ptr;

				XCAssertWithFormattedMessage(ptr, 
					"A memory allocation failed.\n\nRequested chunk size: %llu bytes.\n\nAvail memory: %llu bytes, load (%u%%).",
					size, statex.ullAvailPageFile, statex.dwMemoryLoad);
			}

			return ptr;
		}

		inline static void dealloc(void* block)
		{
			voltek::scalable_free(block);
		}

		inline static size_t msize(void* block)
		{
			return voltek::scalable_msize(block);
		}
	};

	memory_manager g_memory_mgr;

	namespace detail
	{
		class BGSMemoryManager
		{
		public:
			static void* alloc(BGSMemoryManager* self, size_t size, uint32_t alignment, bool aligned)
			{
				return memory_manager::alloc(size, alignment, aligned, true);
			}

			static void dealloc(BGSMemoryManager* self, void* block, bool aligned)
			{
				memory_manager::dealloc(block);
			}

			static void* realloc(BGSMemoryManager* self, void* old_block, size_t size, uint32_t alignment, bool aligned)
			{
				auto new_ptr = memory_manager::alloc(size, alignment, aligned, true);
				if (!new_ptr) return nullptr;

				if (old_block)
				{
					auto old_size = memory_manager::msize(old_block);
					memcpy(new_ptr, old_block, min(old_size, size));
					memory_manager::dealloc(old_block);
				}

				return new_ptr;
			}

			static size_t msize(BGSMemoryManager* self, void* memory)
			{
				return memory_manager::msize(memory);
			}
		};

		class BSSmallBlockAllocator
		{
		public:
			static void* alloc(size_t size, uint32_t alignment, bool aligned)
			{
				return memory_manager::alloc(size, alignment, aligned, false);
			}
			static void sub_nullopt() { return; }
			static int32_t sub08() { return 0; }
			static void* alloc_block(size_t size, uint32_t alignment) { return alloc(size, alignment, true); }
			static void dealloc_block(void* block) { memory_manager::dealloc(block); }
			static void* alloc_block_noalign(size_t size) { return alloc(size, 0, false); }
		};

		class BGSScrapHeap
		{
		public:
			static void* alloc(BGSScrapHeap* manager, size_t size, uint32_t alignment)
			{
				return memory_manager::alloc(size, alignment, alignment != 0);
			}

			static void dealloc(BGSScrapHeap* manager, void* memory)
			{
				memory_manager::dealloc(memory);
			}
		};

		class bhkThreadMemorySource
		{
		private:
			char _pad0[0x8];
			CRITICAL_SECTION m_CritSec;
		public:
			XC_DECLARE_CONSTRUCTOR_HOOK(bhkThreadMemorySource);

			bhkThreadMemorySource()
			{
				InitializeCriticalSection(&m_CritSec);
			}

			virtual ~bhkThreadMemorySource()
			{
				DeleteCriticalSection(&m_CritSec);
			}

			virtual void* blockAlloc(size_t numBytes)
			{
				return memory_manager::alloc(numBytes, 16, true);
			}

			virtual void blockFree(void* p, size_t numBytes)
			{
				memory_manager::dealloc(p);
			}

			virtual void* bufAlloc(size_t& reqNumBytesInOut)
			{
				return blockAlloc(reqNumBytesInOut);
			}

			virtual void bufFree(void* p, size_t numBytes)
			{
				return blockFree(p, numBytes);
			}

			virtual void* bufRealloc(void* pold, size_t oldNumBytes, size_t& reqNumBytesInOut)
			{
				void* p = blockAlloc(reqNumBytesInOut);
				memcpy(p, pold, oldNumBytes);
				blockFree(pold, oldNumBytes);
				return p;
			}

			virtual void blockAllocBatch(void** ptrsOut, size_t numPtrs, size_t blockSize)
			{
				for (long i = 0; i < numPtrs; i++)
					ptrsOut[i] = blockAlloc(blockSize);
			}

			virtual void blockFreeBatch(void** ptrsIn, size_t numPtrs, size_t blockSize)
			{
				for (long i = 0; i < numPtrs; i++)
					blockFree(ptrsIn[i], blockSize);
			}

			virtual void getMemoryStatistics(class MemoryStatistics& u)
			{}

			virtual size_t getAllocatedSize(const void* obj, size_t nbytes)
			{
				return 0;
			}

			virtual void resetPeakMemoryStatistics()
			{}

			virtual void* getExtendedInterface()
			{
				return nullptr;
			}
		};

		class BSScaleformSysMemMapper
		{
		public:
			inline static UInt32 PAGE_SIZE;
			inline static UInt32 HEAP_SIZE;

			static uint32_t get_page_size(BSScaleformSysMemMapper* _this)
			{
				return (uint32_t)PAGE_SIZE;
			}

			static void* init(BSScaleformSysMemMapper* _this, size_t size)
			{
				return VirtualAlloc(NULL, (SIZE_T)size, MEM_RESERVE, PAGE_READWRITE);
			}

			static bool release(BSScaleformSysMemMapper* _this, void* address)
			{
				return VirtualFree((LPVOID)address, (SIZE_T)HEAP_SIZE, MEM_RELEASE);
			}

			static void* alloc(BSScaleformSysMemMapper* _this, void* address, size_t size)
			{
				return VirtualAlloc((LPVOID)address, (SIZE_T)size, MEM_COMMIT, PAGE_READWRITE);
			}

			static bool free(BSScaleformSysMemMapper* _this, void* address, size_t size)
			{
				return VirtualFree((LPVOID)address, (SIZE_T)size, MEM_DECOMMIT);
			}
		};
	}

	static void* impl_calloc(size_t count, size_t size)
	{
		return memory_manager::alloc(count * size, 0);
	}

	static void* impl_malloc(size_t size)
	{
		return memory_manager::alloc(size, 0);
	}

	static void* impl_aligned_malloc(size_t size, size_t alignment)
	{
		return memory_manager::alloc(size, alignment, true);
	}

	static void* impl_realloc(void* memory, size_t size)
	{
		void* newMemory = nullptr;

		if (size > 0)
		{
			newMemory = memory_manager::alloc(size, 0, false);

			if (memory)
				memcpy(newMemory, memory, min(size, voltek::scalable_msize(memory)));
		}

		memory_manager::dealloc(memory);
		return newMemory;
	}

	static void impl_free(void* block)
	{
		memory_manager::dealloc(block);
	}

	static void impl_aligned_free(void* block)
	{
		memory_manager::dealloc(block);
	}

	static size_t impl_msize(void* block)
	{
		return memory_manager::msize(block);
	}

	ModuleMemory::ModuleMemory(void* Context) :
		Module(Context, SourceName, CVarMemory)
	{}

	HRESULT ModuleMemory::InstallImpl()
	{
		if (GetModuleHandleA("BakaScrapHeap.dll"))
		{
			MessageBoxA(0, "Mod \"Baka ScrapHeap\" has been detected. X-Cell "
				"patch \"memory\" is incompatible and will not be enabled.", "Warning", MB_OK | MB_ICONWARNING);
			return S_FALSE;
		}

		MEMORYSTATUSEX statex = { 0 };
		statex.dwLength = sizeof(MEMORYSTATUSEX);
		if (!GlobalMemoryStatusEx(&statex))
			return E_FAIL;

		_MESSAGE("Memory (Total: %.2f Gb, Available: %.2f Gb)",
			((double)statex.ullTotalPageFile / MEM_GB), ((double)statex.ullAvailPageFile / MEM_GB));

		detail::BSScaleformSysMemMapper::PAGE_SIZE = CVarScaleformPageSize->GetUnsignedInt();
		detail::BSScaleformSysMemMapper::HEAP_SIZE = CVarScaleformHeapSize->GetUnsignedInt();

		detail::BSScaleformSysMemMapper::PAGE_SIZE = min(detail::BSScaleformSysMemMapper::PAGE_SIZE, (UInt32)2 * 1024);
		detail::BSScaleformSysMemMapper::PAGE_SIZE = (detail::BSScaleformSysMemMapper::PAGE_SIZE + 7) & ~7;
		detail::BSScaleformSysMemMapper::HEAP_SIZE = min(detail::BSScaleformSysMemMapper::HEAP_SIZE, (UInt32)2 * 1024);
		detail::BSScaleformSysMemMapper::HEAP_SIZE = (detail::BSScaleformSysMemMapper::HEAP_SIZE + 7) & ~7;

		_MESSAGE("BSScaleformSysMemMapper (Page: %u Kb, Heap: %u Mb)",
			detail::BSScaleformSysMemMapper::PAGE_SIZE, detail::BSScaleformSysMemMapper::HEAP_SIZE);

		detail::BSScaleformSysMemMapper::PAGE_SIZE *= 1024;
		detail::BSScaleformSysMemMapper::HEAP_SIZE *= 1024 * 1024;

		auto gContext = (XCell::Context*)Context;
		auto base = gContext->ProcessBase;

		// Replacement of all functions of the standard allocator.

		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "realloc", (UInt64)&impl_realloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc", (UInt64)&impl_calloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc", (UInt64)&impl_aligned_malloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc", (UInt64)&impl_malloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free", (UInt64)&impl_aligned_free);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free", (UInt64)&impl_free);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize", (UInt64)&impl_msize);

		// replacing memory manipulation functions with newer and more productive ones.

		if (CVarUseNewRedistributable->GetBool())
		{
			REL::Impl::DetourIAT(base, "msvcr110.dll", "memcmp", (UInt64)&memcmp);
			REL::Impl::DetourIAT(base, "msvcr110.dll", "memmove", (UInt64)&memmove);
			REL::Impl::DetourIAT(base, "msvcr110.dll", "memcpy", (UInt64)&memcpy);
			REL::Impl::DetourIAT(base, "msvcr110.dll", "memset", (UInt64)&memset);

			if (REL::Version() == RUNTIME_VERSION_1_10_163)
			{
				REL::Impl::DetourIAT(base, "msvcr110.dll", "memmove_s", (UInt64)&memmove_s);
				REL::Impl::DetourIAT(base, "msvcr110.dll", "memcpy_s", (UInt64)&memcpy_s);
			}
		}

		// detail::BGSMemoryManager::alloc pattern 48895C24??48896C24??48897424??5741544155415641574883EC??65488B0425
		// detail::BGSMemoryManager::dealloc pattern 4885D20F84????????48895C24??48895424??574883EC??8039??410FB6F8488BD975??488BCAE9????????488B89????????4885C974??483B51??72??
		// 
		// detail::bhkThreadMemorySource::__ctor__ pattern 488B4F??BA0000040041B90400000041B800100000FF15????????4885C074??

		REL::Impl::DetourJump(REL::ID(30), (UInt64)&detail::BGSMemoryManager::alloc);
		REL::Impl::DetourJump(REL::ID(40), (UInt64)&detail::BGSMemoryManager::dealloc);
		REL::Impl::DetourJump(REL::ID(41), (UInt64)&detail::BGSMemoryManager::realloc);			// NG ONLY
		REL::Impl::DetourJump(REL::ID(50), (UInt64)&detail::BGSMemoryManager::msize);
		REL::Impl::DetourJump(REL::ID(60), (UInt64)&detail::BGSScrapHeap::alloc);
		REL::Impl::DetourJump(REL::ID(70), (UInt64)&detail::BGSScrapHeap::dealloc);
		REL::Impl::DetourJump(REL::ID(80), (UInt64)&detail::bhkThreadMemorySource::__ctor__);	// bhkThreadMemorySource init

		// BSScaleformSysMemMapper
		auto vtable = (uintptr_t*)REL::ID(150);
		if (!vtable)
			return E_FAIL;

		REL::ScopeLock lock((LPVOID)vtable, 0x40);
		vtable[0] = (uintptr_t)detail::BSScaleformSysMemMapper::get_page_size;
		vtable[1] = (uintptr_t)detail::BSScaleformSysMemMapper::init;
		vtable[2] = (uintptr_t)detail::BSScaleformSysMemMapper::release;
		vtable[3] = (uintptr_t)detail::BSScaleformSysMemMapper::alloc;
		vtable[4] = (uintptr_t)detail::BSScaleformSysMemMapper::free;

		REL::Impl::Patch(REL::ID(90), (UInt8*)&detail::BSScaleformSysMemMapper::PAGE_SIZE, 4);
		REL::Impl::Patch(REL::ID(100), (UInt8*)&detail::BSScaleformSysMemMapper::HEAP_SIZE, 4);

		REL::Impl::Patch(REL::ID(110), { 0xC3, 0x90 });	// MemoryManager - Default/Static/File heaps init
		REL::Impl::Patch(REL::ID(120), { 0xC3, 0x90 });	// BSSmallBlockAllocator init
		REL::Impl::Patch(REL::ID(130), { 0xC3, 0x90 });	// ScrapHeap init
		REL::Impl::Patch(REL::ID(140), { 0xC3, 0x90 });	// ScrapHeap deinit

		// Remove BSSmallBlockAllocator
		//
		{
			//
			// Remove the thousands of [code below] since they're useless checks:
			//
			// if ( dword_142E62E00 != 2 ) // MemoryManager initialized flag
			//     sub_14153DDA0((__int64)&unk_142E62980, &dword_142E62E00);
			//
			{
				auto Section = gContext->GetPESectionText();
				auto Matches = REL::Impl::FindPatterns(Section.base, Section.end - Section.base,
					"83 3D ? ? ? ? 02 74 13 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8");

				REL::ScopeLock Lock(Section.base, Section.end - Section.base);
				for (UInt64 match : Matches)
					memcpy((void*)match, "\xEB\x1A", 2);

				_MESSAGE("memory: remove useless checks %llu", Matches.size());
			}

			REL::Impl::Patch(REL::ID(151), { 0xC3, 0x90 }); // NG ONLY / OG CRASHES
		}

		return S_OK;
	}

	HRESULT ModuleMemory::ShutdownImpl()
	{
		// Cannot be disabled in runtime

		return S_FALSE;
	}
}