// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <Voltek.MemoryManager.h>

#include "XCellTableID.h"
#include "XCellModuleMemory.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

#include <xbyak/xbyak.h>
#include <common/ISingleton.h>
#include <array>
#include <tuple>

namespace XCell
{
	constexpr auto MEM_GB = 1073741824;

	namespace detail
	{
		class ICheckerPointer
		{
			ICheckerPointer(const ICheckerPointer&) = delete;
			ICheckerPointer(ICheckerPointer&&) = delete;
			ICheckerPointer& operator=(const ICheckerPointer&) = delete;
			ICheckerPointer& operator=(ICheckerPointer&&) = delete;
		public:
			ICheckerPointer() noexcept(true) = default;
			~ICheckerPointer() noexcept(true) = default;

			inline void* CheckPtr(void* lpBlock, std::size_t nSize) const noexcept(true)
			{
				if (!lpBlock)
				{
					static MEMORYSTATUSEX statex = { 0 };
					statex.dwLength = sizeof(MEMORYSTATUSEX);
					if (!GlobalMemoryStatusEx(&statex))
						return lpBlock;

					XCAssertWithFormattedMessage(lpBlock,
						"A memory allocation failed.\n\nRequested chunk size: %llu bytes.\n\nAvail memory: %llu bytes, load (%u%%).",
						nSize, statex.ullAvailPageFile, statex.dwMemoryLoad);
				}

				return lpBlock;
			}
		};

		class ProxyHeap :
			public ICheckerPointer,
			public ISingleton<ProxyHeap>
		{
			ProxyHeap(const ProxyHeap&) = delete;
			ProxyHeap(ProxyHeap&&) = delete;
			ProxyHeap& operator=(const ProxyHeap&) = delete;
			ProxyHeap& operator=(ProxyHeap&&) = delete;
		public:
			ProxyHeap() noexcept(true) = default;
			~ProxyHeap() noexcept(true) = default;

			[[nodiscard]] void* malloc(std::size_t nSize) const noexcept(true);
			[[nodiscard]] void* aligned_malloc(std::size_t nSize, std::size_t nAlignment) const noexcept(true);

			[[nodiscard]] void* realloc(void* lpBlock, std::size_t nNewSize) const noexcept(true);
			[[nodiscard]] void* aligned_realloc(void* lpBlock, std::size_t nNewSize, std::size_t nAlignment) const noexcept(true);

			void free(void* lpBlock) const noexcept(true);
			void aligned_free(void* lpBlock) const noexcept(true);

			[[nodiscard]] std::size_t msize(void* lpBlock) const noexcept(true);
			[[nodiscard]] std::size_t aligned_msize(void* lpBlock, std::size_t nAlignment) const noexcept(true);
		};

		void* ProxyHeap::malloc(std::size_t nSize) const noexcept(true)
		{
			return CheckPtr(::malloc(nSize), nSize);
		}

		void* ProxyHeap::aligned_malloc(std::size_t nSize, std::size_t nAlignment) const noexcept(true)
		{
			return CheckPtr(_aligned_malloc(nSize, nAlignment), nSize);
		}

		void* ProxyHeap::realloc(void* lpBlock, std::size_t nNewSize) const noexcept(true)
		{
			return CheckPtr(lpBlock ? ::realloc(lpBlock, nNewSize) : ::malloc(nNewSize), nNewSize);
		}

		void* ProxyHeap::aligned_realloc(void* lpBlock, std::size_t nNewSize, std::size_t nAlignment) const noexcept(true)
		{
			return CheckPtr(lpBlock ? ::_aligned_realloc(lpBlock, nNewSize, nAlignment) : _aligned_malloc(nNewSize, nAlignment), nNewSize);
		}

		void ProxyHeap::free(void* lpBlock) const noexcept(true)
		{
			if (lpBlock)
				::free(lpBlock);
		}

		void ProxyHeap::aligned_free(void* lpBlock) const noexcept(true)
		{
			if (lpBlock)
				::_aligned_free(lpBlock);
		}

		std::size_t ProxyHeap::msize(void* lpBlock) const noexcept(true)
		{
			return lpBlock ? ::_msize(lpBlock) : 0;
		}

		std::size_t ProxyHeap::aligned_msize(void* lpBlock, std::size_t nAlignment) const noexcept(true)
		{
			return lpBlock ? ::_aligned_msize(lpBlock, nAlignment, 0) : 0;
		}

		class ProxyVoltekHeap :
			public ICheckerPointer,
			public ISingleton<ProxyVoltekHeap>
		{
			ProxyVoltekHeap(const ProxyVoltekHeap&) = delete;
			ProxyVoltekHeap(ProxyVoltekHeap&&) = delete;
			ProxyVoltekHeap& operator=(const ProxyVoltekHeap&) = delete;
			ProxyVoltekHeap& operator=(ProxyVoltekHeap&&) = delete;
		public:
			ProxyVoltekHeap() noexcept(true);
			~ProxyVoltekHeap() noexcept(true) = default;

			[[nodiscard]] void* malloc(std::size_t nSize) const noexcept(true);
			[[nodiscard]] void* aligned_malloc(std::size_t nSize, std::size_t nAlignment) const noexcept(true);

			[[nodiscard]] void* realloc(void* lpBlock, std::size_t nNewSize) const noexcept(true);
			[[nodiscard]] void* aligned_realloc(void* lpBlock, std::size_t nNewSize, std::size_t nAlignment) const noexcept(true);

			void free(void* lpBlock) const noexcept(true);
			void aligned_free(void* lpBlock) const noexcept(true);

			[[nodiscard]] std::size_t msize(void* lpBlock) const noexcept(true);
			[[nodiscard]] std::size_t aligned_msize(void* lpBlock, std::size_t nAlignment) const noexcept(true);
		};

		ProxyVoltekHeap::ProxyVoltekHeap() noexcept(true)
		{
			voltek::scalable_memory_manager_initialize();
		}
		
		void* ProxyVoltekHeap::malloc(std::size_t nSize) const noexcept(true)
		{
			return CheckPtr(voltek::scalable_alloc(nSize), nSize);
		}

		void* ProxyVoltekHeap::aligned_malloc(std::size_t nSize, std::size_t nAlignment) const noexcept(true)
		{
			UNREFERENCED_PARAMETER(nAlignment);

			return CheckPtr(voltek::scalable_alloc(nSize), nSize);
		}

		void* ProxyVoltekHeap::realloc(void* lpBlock, std::size_t nNewSize) const noexcept(true)
		{
			return CheckPtr(voltek::scalable_realloc(lpBlock, nNewSize), nNewSize);
		}

		void* ProxyVoltekHeap::aligned_realloc(void* lpBlock, std::size_t nNewSize, std::size_t nAlignment) const noexcept(true)
		{
			UNREFERENCED_PARAMETER(nAlignment);

			return CheckPtr(voltek::scalable_realloc(lpBlock, nNewSize), nNewSize);
		}

		void ProxyVoltekHeap::free(void* lpBlock) const noexcept(true)
		{
			voltek::scalable_free(lpBlock);
		}

		void ProxyVoltekHeap::aligned_free(void* lpBlock) const noexcept(true)
		{
			voltek::scalable_free(lpBlock);
		}

		std::size_t ProxyVoltekHeap::msize(void* lpBlock) const noexcept(true)
		{
			return voltek::scalable_msize(lpBlock);
		}

		std::size_t ProxyVoltekHeap::aligned_msize(void* lpBlock, std::size_t nAlignment) const noexcept(true)
		{
			return voltek::scalable_msize(lpBlock);
		}

		template<typename Heap = detail::ProxyHeap>
		struct StdStuff
		{
			[[nodiscard]] static void* calloc(std::size_t nCount, std::size_t nSize) noexcept(true)
			{
				auto totalSize = nCount * nSize;
				auto ptr = Heap::GetSingletonPtr()->malloc(totalSize);
				if (ptr) memset(ptr, 0, totalSize);
				return ptr;
			}

			[[nodiscard]] static void* malloc(std::size_t nSize) noexcept(true)
			{
				return Heap::GetSingletonPtr()->malloc(nSize);
			}

			[[nodiscard]] static void* aligned_malloc(std::size_t nSize, size_t alignment) noexcept(true)
			{
				return Heap::GetSingletonPtr()->aligned_malloc(nSize, alignment);
			}

			[[nodiscard]] static void* realloc(void* lpBlock, std::size_t nNewSize) noexcept(true)
			{
				return Heap::GetSingletonPtr()->realloc(lpBlock, nNewSize);
			}

			static void free(void* block) noexcept(true)
			{
				Heap::GetSingletonPtr()->free(block);
			}

			static void aligned_free(void* block) noexcept(true)
			{
				Heap::GetSingletonPtr()->aligned_free(block);
			}

			[[nodiscard]] static size_t msize(void* block) noexcept(true)
			{
				return Heap::GetSingletonPtr()->msize(block);
			}
		};
	}

	template<typename Heap = detail::ProxyHeap>
	class MemoryManager
	{
		MemoryManager(const MemoryManager&) = delete;
		MemoryManager(MemoryManager&&) = delete;
		MemoryManager& operator=(const MemoryManager&) = delete;
		MemoryManager& operator=(MemoryManager&&) = delete;

		MemoryManager() = default;
		~MemoryManager() = default;
	public:
		inline static const std::uint64_t EMPTY_POINTER{ 0 };

		[[nodiscard]] static void* Alloc(MemoryManager* lpSelf, std::size_t nSize, std::uint32_t nAlignment, bool bAligned) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (!nSize)
				return (void*)(&EMPTY_POINTER);

			return bAligned ? 
				Heap::GetSingletonPtr()->aligned_malloc(nSize, nAlignment) :
				Heap::GetSingletonPtr()->malloc(nSize);
		}

		[[nodiscard]] static void* Realloc(MemoryManager* lpSelf, void* lpBlock, std::size_t nSize, std::uint32_t nAlignment, bool bAligned) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (lpBlock == (const void*)(&EMPTY_POINTER))
				return Alloc(lpSelf, nSize, nAlignment, bAligned);

			return bAligned ?
				Heap::GetSingletonPtr()->aligned_realloc(lpBlock, nSize, nAlignment) :
				Heap::GetSingletonPtr()->realloc(lpBlock, nSize);
		}

		static void Dealloc(MemoryManager* lpSelf, void* lpBlock, bool bAligned) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (lpBlock == (const void*)(&EMPTY_POINTER))
				return;

			if (bAligned)
				Heap::GetSingletonPtr()->aligned_free(lpBlock);		
			else
				Heap::GetSingletonPtr()->free(lpBlock);
		}

		[[nodiscard]] static std::size_t Size(MemoryManager* lpSelf, void* lpBlock) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (lpBlock == (const void*)(&EMPTY_POINTER))
				return 0;

			return Heap::GetSingletonPtr()->msize(lpBlock);
		}

		static void StubInit(XCell::Context* Context)
		{
			auto base = Context->ProcessBase;

			//
			// Remove the thousands of [code below] since they're useless checks:
			//
			// if ( dword_142E62E00 != 2 ) // MemoryManager initialized flag
			//     sub_14153DDA0((__int64)&unk_142E62980, &dword_142E62E00);
			//
			{
				auto Section = Context->GetPESectionText();
				auto Matches = REL::Impl::FindPatterns(Section.base, Section.end - Section.base,
					"83 3D ? ? ? ? 02 74 13 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8");

				REL::ScopeLock Lock(Section.base, Section.end - Section.base);
				for (UInt64 match : Matches)
					memcpy((void*)match, "\xEB\x1A", 2);

				_MESSAGE("memory: remove useless checks %llu", Matches.size());
			}

			REL::Impl::Patch(REL::ID(151), { 0xC3, 0x90 });
			*(std::uint32_t*)REL::ID(152) = 2;
		}

		static void Install()
		{
			using tuple_t = std::tuple<UInt32, void*>;
			const std::array MMPatch
			{
				tuple_t{ 30, &Alloc },
				tuple_t{ 40, &Dealloc },
				tuple_t{ 41, &Realloc },
				tuple_t{ 50, &Size },
			};

			for (const auto& [id, func] : MMPatch)
				REL::Impl::DetourJump(REL::ID(id), (UInt64)func);
		}
	};

	class AutoScrapHeap
	{
		AutoScrapHeap(const AutoScrapHeap&) = delete;
		AutoScrapHeap(AutoScrapHeap&&) = delete;
		AutoScrapHeap& operator=(const AutoScrapHeap&) = delete;
		AutoScrapHeap& operator=(AutoScrapHeap&&) = delete;

		AutoScrapHeap() = default;
		~AutoScrapHeap() = default;

		inline static void CtorLong()
		{
			REL::Impl::PatchNop(REL::ID(51) + 0x1D, 0x15);
		}

		static void CtorShort()
		{
			struct Patch :
				Xbyak::CodeGenerator
			{
				Patch()
				{
					mov(qword[rcx], 0);
					mov(rax, rcx);
					ret();
				}
			} p;

			auto Off = REL::ID(52);

			p.ready();
			XCAssert(p.getSize() <= 0x1C);

			REL::Impl::PatchNop(Off, 0x1C);
			REL::Impl::Patch(Off, p.getCode<UInt8*>(), p.getSize());
		}

		static void Dtor()
		{
			struct Patch :
				Xbyak::CodeGenerator
			{
				Patch()
				{
					xor_(rax, rax);
					cmp(rbx, rax);
				}
			} p;

			auto Off = REL::ID(53);
			p.ready();
			XCAssert(p.getSize() <= 0x1D);

			REL::Impl::PatchNop(Off + 0x9, 0x1D);
			REL::Impl::Patch(Off + 0x9, p.getCode<UInt8*>(), p.getSize());
			REL::Impl::Patch(Off + 0x26, { 0x74 }); // jnz -> jz
		}
	public:
		static void Install()
		{
			REL::Impl::Patch(REL::ID(120), { 0xC3, 0x90, 0x90, 0x90 });

			CtorLong();
			CtorShort();
			Dtor();
		}
	};

	template<typename Heap = detail::ProxyHeap>
	class ScrapHeap
	{
		ScrapHeap(const ScrapHeap&) = delete;
		ScrapHeap(ScrapHeap&&) = delete;
		ScrapHeap& operator=(const ScrapHeap&) = delete;
		ScrapHeap& operator=(ScrapHeap&&) = delete;

		ScrapHeap() = default;
		~ScrapHeap() = default;

		static void WriteStubs()
		{
			// Remove stuff

			REL::Impl::Patch(REL::ID(140), { 0xC3, 0x90, 0x90, 0x90 });	// Clean
			REL::Impl::Patch(REL::ID(62), { 0xC3, 0x90, 0x90, 0x90 });	// ClearKeepPages
			REL::Impl::Patch(REL::ID(63), { 0xC3, 0x90, 0x90, 0x90 });	// InsertFreeBlock
			REL::Impl::Patch(REL::ID(64), { 0xC3, 0x90, 0x90, 0x90 });	// RemoveFreeBlock
			REL::Impl::Patch(REL::ID(65), { 0xC3, 0x90, 0x90, 0x90 });	// SetKeepPages
			REL::Impl::Patch(REL::ID(66), { 0xC3, 0x90, 0x90, 0x90 });	// dtor
			REL::Impl::Patch(REL::ID(130), { 0xC3, 0x90, 0x90, 0x90 });	// ctor
		}

		static void WriteHooks()
		{
			using tuple_t = std::tuple<std::uint64_t, void*>;
			const std::array MMPatch
			{
				tuple_t{ 60, &Allocate },
				tuple_t{ 70, &Deallocate },
			};

			for (const auto& [id, func] : MMPatch)
				REL::Impl::DetourJump(REL::ID(id), (UInt64)func);
		}
	public:
		inline static const std::uint64_t EMPTY_POINTER{ 0 };

		[[nodiscard]] inline static void* Allocate(ScrapHeap* lpSelf, std::size_t nSize, std::size_t nAlignment) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (!nSize)
				return (void*)(&EMPTY_POINTER);

			return Heap::GetSingletonPtr()->aligned_malloc(nSize, nAlignment);
		}

		inline static void Deallocate(ScrapHeap* lpSelf, void* lpBlock) noexcept(true)
		{
			UNREFERENCED_PARAMETER(lpSelf);

			if (lpBlock == (const void*)(&EMPTY_POINTER))
				return;

			Heap::GetSingletonPtr()->aligned_free(lpBlock);
		}

		static void Install()
		{
			WriteStubs();
			WriteHooks();
		}
	};

	template<typename Heap = detail::ProxyHeap>
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

		[[nodiscard]] virtual void* blockAlloc(std::size_t numBytes)
		{
			return Heap::GetSingletonPtr()->aligned_malloc(numBytes, 16);
		}

		virtual void blockFree(void* p, std::size_t numBytes)
		{
			Heap::GetSingletonPtr()->aligned_free(p);
		}

		[[nodiscard]] virtual void* bufAlloc(std::size_t& reqNumBytesInOut)
		{
			return blockAlloc(reqNumBytesInOut);
		}

		virtual void bufFree(void* p, std::size_t numBytes)
		{
			return blockFree(p, numBytes);
		}

		[[nodiscard]] virtual void* bufRealloc(void* pold, std::size_t oldNumBytes, std::size_t& reqNumBytesInOut)
		{
			void* p = blockAlloc(reqNumBytesInOut);
			memcpy(p, pold, oldNumBytes);
			blockFree(pold, oldNumBytes);
			return p;
		}

		virtual void blockAllocBatch(void** ptrsOut, std::size_t numPtrs, std::size_t blockSize)
		{
			for (long i = 0; i < numPtrs; i++)
				ptrsOut[i] = blockAlloc(blockSize);
		}

		virtual void blockFreeBatch(void** ptrsIn, std::size_t numPtrs, std::size_t blockSize)
		{
			for (long i = 0; i < numPtrs; i++)
				blockFree(ptrsIn[i], blockSize);
		}

		virtual void getMemoryStatistics(class MemoryStatistics& u)
		{}

		virtual std::size_t getAllocatedSize(const void* obj, std::size_t nbytes)
		{
			return 0;
		}

		virtual void resetPeakMemoryStatistics()
		{}

		[[nodiscard]] virtual void* getExtendedInterface()
		{
			return nullptr;
		}
	};

	template<typename Heap = detail::ProxyHeap>
	class BSSmallBlockAllocator
	{
		BSSmallBlockAllocator(const BSSmallBlockAllocator&) = delete;
		BSSmallBlockAllocator(BSSmallBlockAllocator&&) = delete;
		BSSmallBlockAllocator& operator=(const BSSmallBlockAllocator&) = delete;
		BSSmallBlockAllocator& operator=(BSSmallBlockAllocator&&) = delete;

		BSSmallBlockAllocator() = default;
		~BSSmallBlockAllocator() = default;
	public:
		//[[nodiscard]] inline static void* Allocate(std::size_t nSize)
		//{
		//	return Heap::GetSingletonPtr()->aligned_malloc(nSize, 16);
		//}

		//inline static void Deallocate(void* lpBlock)
		//{
		//	Heap::GetSingletonPtr()->aligned_free(lpBlock);
		//}

		static void Install()
		{
			if (REL::Version() == RUNTIME_VERSION_1_10_163)
				REL::Impl::PatchNop(REL::ID(141), 5);
			else
				REL::Impl::Patch(REL::ID(141), { 0xEB });
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

	ModuleMemory::ModuleMemory(void* Context) :
		Module(Context, SourceName, CVarMemory)
	{}

	HRESULT ModuleMemory::InstallImpl()
	{
		MEMORYSTATUSEX statex = { 0 };
		statex.dwLength = sizeof(MEMORYSTATUSEX);
		if (!GlobalMemoryStatusEx(&statex))
			return E_FAIL;

		_MESSAGE("Memory (Total: %.2f Gb, Available: %.2f Gb)",
			((double)statex.ullTotalPageFile / MEM_GB), ((double)statex.ullAvailPageFile / MEM_GB));

		/////////////////////////////////////////////////////////////////////
		// Replacement of all functions of the standard allocator
		/////////////////////////////////////////////////////////////////////

		auto base = Context->ProcessBase;
		auto gContext = (XCell::Context*)Context;

		// Init vmm
		detail::ProxyVoltekHeap heap;

		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "realloc", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::realloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::calloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::aligned_malloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::malloc);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::aligned_free);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::free);
		REL::Impl::DetourIAT(base, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize", (UInt64)&detail::StdStuff<detail::ProxyVoltekHeap>::msize);
		
		/////////////////////////////////////////////////////////////////////
		// MemoryManager
		/////////////////////////////////////////////////////////////////////

		MemoryManager<detail::ProxyVoltekHeap>::StubInit(gContext);
		MemoryManager<detail::ProxyVoltekHeap>::Install();

		/////////////////////////////////////////////////////////////////////
		// AutoScrapHeap & ScrapHeap
		/////////////////////////////////////////////////////////////////////

		AutoScrapHeap::Install();
		ScrapHeap<detail::ProxyVoltekHeap>::Install();

		/////////////////////////////////////////////////////////////////////
		// bhkThreadMemorySource
		/////////////////////////////////////////////////////////////////////

		REL::Impl::DetourJump(REL::ID(80), (UInt64)&bhkThreadMemorySource<detail::ProxyVoltekHeap>::__ctor__);

		/////////////////////////////////////////////////////////////////////
		// Replacing memory manipulation functions with newer and more productive ones
		/////////////////////////////////////////////////////////////////////

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

		/////////////////////////////////////////////////////////////////////
		// Default/Static/File heaps
		/////////////////////////////////////////////////////////////////////

		REL::Impl::Patch(REL::ID(110), { 0xC3, 0x90 });	
		
		/////////////////////////////////////////////////////////////////////
		// BSSmallBlockAllocator
		/////////////////////////////////////////////////////////////////////

		BSSmallBlockAllocator<detail::ProxyVoltekHeap>::Install();

		/////////////////////////////////////////////////////////////////////
		// BSScaleformSysMemMapper
		/////////////////////////////////////////////////////////////////////

		BSScaleformSysMemMapper::PAGE_SIZE = CVarScaleformPageSize->GetUnsignedInt();
		BSScaleformSysMemMapper::HEAP_SIZE = CVarScaleformHeapSize->GetUnsignedInt();

		BSScaleformSysMemMapper::PAGE_SIZE = std::min(BSScaleformSysMemMapper::PAGE_SIZE, (UInt32)2 * 1024);
		BSScaleformSysMemMapper::PAGE_SIZE = (BSScaleformSysMemMapper::PAGE_SIZE + 7) & ~7;
		BSScaleformSysMemMapper::HEAP_SIZE = std::min(BSScaleformSysMemMapper::HEAP_SIZE, (UInt32)2 * 1024);
		BSScaleformSysMemMapper::HEAP_SIZE = (BSScaleformSysMemMapper::HEAP_SIZE + 7) & ~7;

		_MESSAGE("BSScaleformSysMemMapper (Page: %u Kb, Heap: %u Mb)",
			BSScaleformSysMemMapper::PAGE_SIZE, BSScaleformSysMemMapper::HEAP_SIZE);

		BSScaleformSysMemMapper::PAGE_SIZE *= 1024;
		BSScaleformSysMemMapper::HEAP_SIZE *= 1024 * 1024;

		auto vtable = (uintptr_t*)REL::ID(150);
		if (!vtable)
			return E_FAIL;

		REL::ScopeLock lock((LPVOID)vtable, 0x40);
		vtable[0] = (std::uintptr_t)BSScaleformSysMemMapper::get_page_size;
		vtable[1] = (std::uintptr_t)BSScaleformSysMemMapper::init;
		vtable[2] = (std::uintptr_t)BSScaleformSysMemMapper::release;
		vtable[3] = (std::uintptr_t)BSScaleformSysMemMapper::alloc;
		vtable[4] = (std::uintptr_t)BSScaleformSysMemMapper::free;

		REL::Impl::Patch(REL::ID(90), (UInt8*)&BSScaleformSysMemMapper::PAGE_SIZE, 4);
		REL::Impl::Patch(REL::ID(100), (UInt8*)&BSScaleformSysMemMapper::HEAP_SIZE, 4);

		return S_OK;
	}

	HRESULT ModuleMemory::ShutdownImpl()
	{
		// Cannot be disabled in runtime

		return S_FALSE;
	}
}