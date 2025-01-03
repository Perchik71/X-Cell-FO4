﻿// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <Voltek.MemoryManager.h>
#include <ms_rtti.h>
#include <xc_assertion.h>
#include <xc_patch_memory.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	constexpr auto MEM_GB = 1073741824;

	class memory_manager
	{
	public:
		memory_manager()
		{
			// Инициализация библиотеки vmm
			voltek::scalable_memory_manager_initialize();
		}

		memory_manager(const memory_manager&) = default;
		memory_manager& operator=(const memory_manager&) = default;

		static void* alloc(size_t size, size_t alignment, bool aligned = false, bool zeroed = true)
		{
			// Если не задано, то будет 4
			if (!aligned)
				alignment = 4;

			if (!size)
				return voltek::scalable_alloc(0);

			_xc_assert_msg_fmt((alignment != 0 && alignment % 2 == 0), "Alignment is fucked: %llu", alignment);

			// Должно быть в степени 2, округлить его, если необходимо
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

			// Размер должен быть кратен выравниванию с округлением до ближайшего
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

				_xc_assert_msg_fmt(ptr, "A memory allocation failed.\n\nRequested chunk size: %llu bytes.\n\nAvail memory: %llu bytes, load (%u%%).",
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

	const char* patch_memory::get_name() const noexcept
	{
		return "memory";
	}

	bool patch_memory::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool patch_memory::run() const
	{
		if (GetModuleHandleA("BakaScrapHeap.dll"))
		{
			MessageBoxA(0, "Mod \"Baka ScrapHeap\" has been detected. X-Cell "
				"patch \"memory\" is incompatible and will not be enabled.", "Warning", MB_OK | MB_ICONWARNING);
			return false;
		}

		auto base = GetModuleHandleA(NULL);

		MEMORYSTATUSEX statex = { 0 };
		statex.dwLength = sizeof(MEMORYSTATUSEX);
		if (!GlobalMemoryStatusEx(&statex))
			return false;

		_MESSAGE("Memory (Total: %.2f Gb, Available: %.2f Gb)",
			((double)statex.ullTotalPageFile / MEM_GB), ((double)statex.ullAvailPageFile / MEM_GB));

		detail::BSScaleformSysMemMapper::PAGE_SIZE = g_plugin->read_setting_uint("additional", "scaleform_page_size", 256 /* 256 Kb */);
		detail::BSScaleformSysMemMapper::HEAP_SIZE = g_plugin->read_setting_uint("additional", "scaleform_heap_size", 512 /* 512 Mb */);

		detail::BSScaleformSysMemMapper::PAGE_SIZE = min(detail::BSScaleformSysMemMapper::PAGE_SIZE, (UInt32)2 * 1024);
		detail::BSScaleformSysMemMapper::PAGE_SIZE = (detail::BSScaleformSysMemMapper::PAGE_SIZE + 7) & ~7;
		detail::BSScaleformSysMemMapper::HEAP_SIZE = min(detail::BSScaleformSysMemMapper::HEAP_SIZE, (UInt32)2 * 1024);
		detail::BSScaleformSysMemMapper::HEAP_SIZE = (detail::BSScaleformSysMemMapper::HEAP_SIZE + 7) & ~7;

		_MESSAGE("BSScaleformSysMemMapper (Page: %u Kb, Heap: %u Mb)",
			detail::BSScaleformSysMemMapper::PAGE_SIZE, detail::BSScaleformSysMemMapper::HEAP_SIZE);

		detail::BSScaleformSysMemMapper::PAGE_SIZE *= 1024;
		detail::BSScaleformSysMemMapper::HEAP_SIZE *= 1024 * 1024;

		// Replacement of all functions of the standard allocator.

		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "realloc", (uintptr_t)&impl_realloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc", (uintptr_t)&impl_calloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc", (uintptr_t)&impl_aligned_malloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc", (uintptr_t)&impl_malloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free", (uintptr_t)&impl_aligned_free);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free", (uintptr_t)&impl_free);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize", (uintptr_t)&impl_msize);

		// replacing memory manipulation functions with newer and more productive ones.
		if (g_plugin->read_setting_bool("additional", "use_new_redistributable", false))
		{
			if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
			{
				patch_iat(base, "msvcr110.dll", "memcmp", (uintptr_t)&memcmp);
				patch_iat(base, "msvcr110.dll", "memmove", (uintptr_t)&memmove);
				patch_iat(base, "msvcr110.dll", "memcpy", (uintptr_t)&memcpy);
				patch_iat(base, "msvcr110.dll", "memset", (uintptr_t)&memset);
				patch_iat(base, "msvcr110.dll", "memmove_s", (uintptr_t)&memmove_s);
				patch_iat(base, "msvcr110.dll", "memcpy_s", (uintptr_t)&memcpy_s);
			}
			else
			{
				patch_iat(base, "vcruntime140.dll", "memcmp", (uintptr_t)&memcmp);
				patch_iat(base, "vcruntime140.dll", "memmove", (uintptr_t)&memmove);
				patch_iat(base, "vcruntime140.dll", "memcpy", (uintptr_t)&memcpy);
				patch_iat(base, "vcruntime140.dll", "memset", (uintptr_t)&memset);
			}
		}

		// detail::BGSMemoryManager::alloc pattern 48895C24??48896C24??48897424??5741544155415641574883EC??65488B0425
		// detail::BGSMemoryManager::dealloc pattern 4885D20F84????????48895C24??48895424??574883EC??8039??410FB6F8488BD975??488BCAE9????????488B89????????4885C974??483B51??72??
		// 
		// detail::bhkThreadMemorySource::__ctor__ pattern 488B4F??BA0000040041B90400000041B800100000FF15????????4885C074??

		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// 163
			detour_jump((g_plugin->get_base() + 0x1B0EFD0), (uintptr_t)&detail::BGSMemoryManager::alloc);
			detour_jump((g_plugin->get_base() + 0x1B0F2E0), (uintptr_t)&detail::BGSMemoryManager::dealloc);
			detour_jump((g_plugin->get_base() + 0x1B0E7D0), (uintptr_t)&detail::BGSMemoryManager::msize);
			detour_jump((g_plugin->get_base() + 0x1B13F70), (uintptr_t)&detail::BGSScrapHeap::alloc);
			detour_jump((g_plugin->get_base() + 0x1B14580), (uintptr_t)&detail::BGSScrapHeap::dealloc);
			detour_jump((g_plugin->get_base() + 0x1E21B10), (uintptr_t)&detail::bhkThreadMemorySource::__ctor__);	// bhkThreadMemorySource init

			// BSScaleformSysMemMapper
			{
				auto vtable = (uintptr_t*)(g_plugin->get_base() + 0x2EB92C8);
				scope_relocate_al lock((LPVOID)vtable, 0x40);
				vtable[0] = (uintptr_t)detail::BSScaleformSysMemMapper::get_page_size;
				vtable[1] = (uintptr_t)detail::BSScaleformSysMemMapper::init;
				vtable[2] = (uintptr_t)detail::BSScaleformSysMemMapper::release;
				vtable[3] = (uintptr_t)detail::BSScaleformSysMemMapper::alloc;
				vtable[4] = (uintptr_t)detail::BSScaleformSysMemMapper::free;

				patch_mem((g_plugin->get_base() + 0x211214B), (UInt8*)&detail::BSScaleformSysMemMapper::PAGE_SIZE, 4);
				patch_mem((g_plugin->get_base() + 0x2112151), (UInt8*)&detail::BSScaleformSysMemMapper::HEAP_SIZE, 4);
			}

			// So that it is never called
			patch_mem((g_plugin->get_base() + 0xD0C160), { 0xC3, 0x90 });	// MemoryManager - Default/Static/File heaps init
			patch_mem((g_plugin->get_base() + 0x1B0EDB0), { 0xC3, 0x90 });	// BSSmallBlockAllocator init
			patch_mem((g_plugin->get_base() + 0x1B13DF0), { 0xC3, 0x90 });	// ScrapHeap init
			patch_mem((g_plugin->get_base() + 0x1B14740), { 0xC3, 0x90 });	// ScrapHeap deinit
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// 984
			detour_jump((g_plugin->get_base() + 0x153D7D0), (uintptr_t)&detail::BGSMemoryManager::alloc);
			detour_jump((g_plugin->get_base() + 0x153DC30), (uintptr_t)&detail::BGSMemoryManager::dealloc);
			detour_jump((g_plugin->get_base() + 0x153DA30), (uintptr_t)&detail::BGSMemoryManager::realloc);
			detour_jump((g_plugin->get_base() + 0x153CFA0), (uintptr_t)&detail::BGSMemoryManager::msize);
			detour_jump((g_plugin->get_base() + 0x1542010), (uintptr_t)&detail::BGSScrapHeap::alloc);
			detour_jump((g_plugin->get_base() + 0x15425E0), (uintptr_t)&detail::BGSScrapHeap::dealloc);
			detour_jump((g_plugin->get_base() + 0x17D9DF0), (uintptr_t)&detail::bhkThreadMemorySource::__ctor__);	// bhkThreadMemorySource init

			// BSScaleformSysMemMapper
			{
				auto vtable = (uintptr_t*)(g_plugin->get_base() + 0x25131D8);
				scope_relocate_al lock((LPVOID)vtable, 0x40);
				vtable[0] = (uintptr_t)detail::BSScaleformSysMemMapper::get_page_size;
				vtable[1] = (uintptr_t)detail::BSScaleformSysMemMapper::init;
				vtable[2] = (uintptr_t)detail::BSScaleformSysMemMapper::release;
				vtable[3] = (uintptr_t)detail::BSScaleformSysMemMapper::alloc;
				vtable[4] = (uintptr_t)detail::BSScaleformSysMemMapper::free;

				patch_mem((g_plugin->get_base() + 0x19FF5D9), (UInt8*)&detail::BSScaleformSysMemMapper::PAGE_SIZE, 4);
				patch_mem((g_plugin->get_base() + 0x19FF5E4), (UInt8*)&detail::BSScaleformSysMemMapper::HEAP_SIZE, 4);
			}

			// So that it is never called
			patch_mem((g_plugin->get_base() + 0xB8DC50), { 0xC3, 0x90 });	// MemoryManager - Default/Static/File heaps init
			patch_mem((g_plugin->get_base() + 0x153D5D0), { 0xC3, 0x90 });	// BSSmallBlockAllocator init
			patch_mem((g_plugin->get_base() + 0x1541E90), { 0xC3, 0x90 });	// ScrapHeap init
			patch_mem((g_plugin->get_base() + 0x15427A0), { 0xC3, 0x90 });	// ScrapHeap deinit
		}
		else
			_ERROR("The patch has not been fully installed, as the mod does not know the game");

		// Remove BSSmallBlockAllocator
		//
		{
			//
			// Remove the thousands of [code below] since they're useless checks:
			//
			// if ( dword_142E62E00 != 2 ) // MemoryManager initialized flag
			//     sub_14153DDA0((__int64)&unk_142E62980, &dword_142E62E00);
			//

			auto sec = g_plugin->get_section(0);
			auto matches = find_patterns(sec.base, sec.end - sec.base,
				"83 3D ? ? ? ? 02 74 13 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8");

			uint32_t f;
			unlock_page(sec.base, sec.end - sec.base, f);
			for (uintptr_t match : matches)
				memcpy((void*)match, "\xEB\x1A", 2);
			lock_page(sec.base, sec.end - sec.base, f);

			_MESSAGE("memory: remove useless checks %llu", matches.size());

			if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
				patch_mem((g_plugin->get_base() + 0x153DDA0), { 0xC3, 0x90 });
			// crashes
			//else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
			//	patch_mem((g_plugin->get_base() + 0x1B0F450), { 0xC3, 0x90 });
		}

		return true;
	}

	void* patch_memory::impl_calloc(size_t count, size_t size)
	{
		return memory_manager::alloc(count * size, 0);
	}

	void* patch_memory::impl_malloc(size_t size)
	{
		return memory_manager::alloc(size, 0);
	}

	void* patch_memory::impl_aligned_malloc(size_t size, size_t alignment)
	{
		return memory_manager::alloc(size, alignment, true);
	}

	void* patch_memory::impl_realloc(void* memory, size_t size)
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

	void patch_memory::impl_free(void* block)
	{
		memory_manager::dealloc(block);
	}

	void patch_memory::impl_aligned_free(void* block)
	{
		memory_manager::dealloc(block);
	}

	size_t patch_memory::impl_msize(void* block)
	{
		return memory_manager::msize(block);
	}
}