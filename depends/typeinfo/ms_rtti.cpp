// Author: Nukem9
// https://github.com/Nukem9/SkyrimSETest/blob/master/skyrim64_test/src/typeinfo/ms_rtti.cpp

#include "ms_rtti.h"

extern "C"
{
	typedef void* (*malloc_func_t)(size_t);
	typedef void(*free_func_t)(void*);
	char *__unDNameEx(char *outputString, const char *name, int maxStringLength, malloc_func_t pAlloc, free_func_t pFree, char *(__fastcall *pGetParameter)(int), unsigned int disableFlags);
}

namespace msrtti
{
	using namespace detail;

	vector<info> Tables;
	uintptr_t g_RdataBase, g_RdataEnd, g_DataBase, g_DataEnd, g_CodeBase, g_CodeEnd;
	uintptr_t g_ModuleBase;

	void init(uintptr_t base, const section& code, const section& data, const section& rdata)
	{
		g_ModuleBase = base;
		g_RdataBase = rdata.base;
		g_RdataEnd = rdata.end;	
		g_DataBase = data.base;
		g_DataEnd = data.end;
		g_CodeBase = code.base;
		g_CodeEnd = code.end;

		for (uintptr_t i = g_RdataBase; i < (g_RdataEnd - sizeof(uintptr_t) - sizeof(uintptr_t)); i++)
		{
			// Skip all non-2-aligned addresses. Not sure if this is OK or it skips tables.
			if (i % 2 != 0)
				continue;

			//
			// This might be a valid RTTI entry, so check if:
			// - The COL points to somewhere in .rdata
			// - The COL has a valid signature
			// - The first virtual function points to .text
			//
			uintptr_t addr = *(uintptr_t*)i;
			uintptr_t vfuncAddr = *(uintptr_t*)(i + sizeof(uintptr_t));

			if (!is_within_rdata(addr) || !is_within_code(vfuncAddr))
				continue;

			auto locator = reinterpret_cast<complete_object_locator*>(addr);

			if (!is_valid_col(locator))
				continue;

			info _info;
			_info.vtable_address = i + sizeof(uintptr_t);
			_info.vtable_offset = locator->offset;
			_info.vfunction_total = 0;
			_info.raw_name = locator->type_desc.get()->name;
			_info.locator = locator;

			// Demangle
			_info.name = __unDNameEx(nullptr, _info.raw_name + 1, 0, malloc, free, nullptr, 0x2800);

			// Determine number of virtual functions
			for (uintptr_t j = _info.vtable_address; j < (g_RdataEnd - sizeof(uintptr_t)); j += sizeof(uintptr_t))
			{
				if (!is_within_code(*(uintptr_t*)j))
					break;

				_info.vfunction_total++;
			}

			// Done
			Tables.push_back(_info);
		}
	}

	void dump(FILE* file)
	{
		for (const info& _info : Tables)
			fprintf(file, "`%s`: VTable [0x%p, 0x%p offset, %lld functions] `%s`\n", _info.name,
				_info.vtable_address - g_ModuleBase, _info.vtable_offset, _info.vfunction_total, _info.raw_name);
	}

	const info* find(const char* name, bool exact)
	{
		auto results = find_all(name, exact);
		if (results.size() > 1) return nullptr;
		return results.at(0);
	}

	vector<const info*> find_all(const char* name, bool exact)
	{
		// Multiple classes can have identical names but different vtable displacements,
		// so return all that match
		vector<const info*> results;

		for (const info& _info : Tables)
		{
			if (exact)
			{
				if (!strcmp(_info.name, name))
					results.push_back(&_info);
			}
			else
			{
				if (strcasestr(_info.name, name))
					results.push_back(&_info);
			}
		}

		return results;
	}

	namespace detail
	{
		bool is_within_rdata(uintptr_t address)
		{
			return (address >= g_RdataBase && address <= g_RdataEnd) ||
				(address >= g_DataBase && address <= g_DataEnd);
		}

		bool is_within_code(uintptr_t address)
		{
			return address >= g_CodeBase && address <= g_CodeEnd;
		}

		bool is_valid_col(complete_object_locator* locator)
		{
			return locator->signature == complete_object_locator::col_signature_64 && is_within_rdata(locator->type_desc.address());
		}

		const char* strcasestr(const char* string, const char* substring)
		{
			const char* a, * b;

			for (; *string; *string++)
			{
				a = string;
				b = substring;

				while (toupper(*a++) == toupper(*b++))
					if (!*b)
						return string;
			}

			return nullptr;
		}
	}
}