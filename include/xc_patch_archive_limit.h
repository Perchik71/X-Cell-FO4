// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_archive_limit : public patch
	{
	public:
		struct file_hash_t
		{
			uint32_t	hash_name;
			char		ext[4];
			uint32_t	unk;		// maybe offset

			union parms_t
			{
				struct original_parms_t
				{
					uint8_t archive_index;
					uint8_t is_chunk;
					uint16_t chunk_size;
				} original_parms;

				// Since there is always a chunk and it is 16 bytes, it is appropriate to shift this information by a byte.
				// Increasing the number of valid indexes in the hash.

				struct modded_parms_t
				{
					uint16_t archive_index;	
					uint8_t is_chunk;
					uint8_t chunk_size;
				} modded_parms;
			} parms;
		};
	public:
		patch_archive_limit() = default;
		patch_archive_limit(const patch_archive_limit&) = default;
		virtual ~patch_archive_limit() = default;

		patch_archive_limit& operator=(const patch_archive_limit&) = default;

		virtual const char* get_name() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static uint32_t impl_memcpy_hash_from_archive_table(void* archive, void* archive_hash, file_hash_t* hash, size_t read_size);
		static void impl_set_index_archive_to_hash();
	};
}