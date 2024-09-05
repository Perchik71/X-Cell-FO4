// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_memory : public patch
	{
	public:
		patch_memory() = default;
		patch_memory(const patch_memory&) = default;
		virtual ~patch_memory() = default;

		patch_memory& operator=(const patch_memory&) = default;

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static void* impl_calloc(size_t count, size_t size);
		static void* impl_malloc(size_t size);
		static void* impl_aligned_malloc(size_t size, size_t alignment);
		static void* impl_realloc(void* memory, size_t size);
		static void impl_free(void* block);
		static void impl_aligned_free(void* block);
		static size_t impl_msize(void* block);
	};
}