// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_patch_archive_limit.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	uintptr_t memcpy_hash_from_archive_table_orig = 0;

	const char* patch_archive_limit::get_name() const noexcept
	{
		return "archive_limit";
	}

	bool patch_archive_limit::run() const
	{
		detour_call(g_plugin->get_base() + 0x158646F, (uintptr_t)&impl_memcpy_hash_from_archive_table);
		memcpy_hash_from_archive_table_orig = g_plugin->get_base() + 0x1587BA0;

		detour_call(g_plugin->get_base() + 0x15864B5, (uintptr_t)&impl_set_index_archive_to_hash);

		auto offset = g_plugin->get_base() + 1587095;
		// Remove useless stuff.	
		patch_mem_nop(offset, 0x16);
		// mov eax, dword ptr ds:[rsi+0xC]
		// mov dword ptr ds:[rdi+0xC], eax
		patch_mem(offset, { 0x8B, 0x46, 0x0C, 0x89, 0x47, 0x0C });

		return true;
	}

	uint32_t patch_archive_limit::impl_memcpy_hash_from_archive_table(void* archive, void* archive_hash, file_hash_t* hash, size_t read_size)
	{
		auto result = fastCall<uint32_t>(memcpy_hash_from_archive_table_orig, archive, archive_hash, hash, read_size);
		
		file_hash_t::parms_t::original_parms_t parms = hash->parms.original_parms;

		hash->parms.modded_parms.archive_index = 0;
		hash->parms.modded_parms.is_chunk = parms.is_chunk;
		hash->parms.modded_parms.chunk_size = parms.chunk_size;

		return result;	// 0 - OK
	}

	void patch_archive_limit::impl_set_index_archive_to_hash()
	{
		// It is necessary to get the stack of the calling function.
		auto rsp = (uintptr_t)_AddressOfReturnAddress() + 8;
		// Set archive index from stack
		*((uint16_t*)(rsp + 0x3C)) = *((uint16_t*)(rsp + 0x1E8));
	}
}