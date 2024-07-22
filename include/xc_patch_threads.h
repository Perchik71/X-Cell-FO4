// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_threads : public patch
	{
	public:
		patch_threads() = default;
		patch_threads(const patch_threads&) = default;
		virtual ~patch_threads() = default;

		patch_threads& operator=(const patch_threads&) = default;

		virtual const char* get_name() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static uint32_t sleep(uint32_t ms);
		static uint32_t sleep_ex(uint32_t ms, bool alterable);
		static bool set_thread_priority(HANDLE thread_handle, int priority);
		static uintptr_t set_thread_affinity_mask(HANDLE thread_handle, uintptr_t affinity_mask);
	};
}