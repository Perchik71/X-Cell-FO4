// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <vmm.h>
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

			// _xc_assert_msg_fmt(ptr, "A memory allocation failed.\n\nRequested chunk size: %llu bytes.", size);

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
	}

	const char* patch_memory::get_name() const noexcept
	{
		return "memory";
	}

	bool patch_memory::run() const
	{
		auto base = GetModuleHandleA(NULL);

		MEMORYSTATUSEX statex = { 0 };
		statex.dwLength = sizeof(MEMORYSTATUSEX);
		if (!GlobalMemoryStatusEx(&statex)) 
			return false;
	
		_MESSAGE("Memory (Total: %.2f Gb, Available: %.2f Gb)", 
			((double)statex.ullTotalPageFile / MEM_GB), ((double)statex.ullAvailPageFile / MEM_GB));
		
		// Replacement of all functions of the standard allocator.

		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "realloc", (uintptr_t)&impl_realloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc", (uintptr_t)&impl_calloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc", (uintptr_t)&impl_aligned_malloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc", (uintptr_t)&impl_malloc);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free", (uintptr_t)&impl_aligned_free);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free", (uintptr_t)&impl_free);
		patch_iat(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize", (uintptr_t)&impl_msize);

		// replacing memory manipulation functions with newer and more productive ones.

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

			// So that it is never called
			patch_mem((g_plugin->get_base() + 0xB8DC50), { 0xC3, 0x90 });	// MemoryManager - Default/Static/File heaps init
			patch_mem((g_plugin->get_base() + 0x153D5D0), { 0xC3, 0x90 });	// BSSmallBlockAllocator init
			patch_mem((g_plugin->get_base() + 0x1541E90), { 0xC3, 0x90 });	// ScrapHeap init
			patch_mem((g_plugin->get_base() + 0x15427A0), { 0xC3, 0x90 });	// ScrapHeap deinit
		}
		else
			_ERROR("The patch has not been fully installed, as the mod does not know the game");
		
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