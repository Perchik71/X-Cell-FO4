// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <initializer_list>

namespace xc
{
	class patch
	{
	public:
		patch() = default;
		patch(const patch&) = default;
		virtual ~patch() = default;

		patch& operator=(const patch&) = default;

		bool start();

		virtual const char* get_name() const noexcept = 0;
	protected:
		virtual bool run() const = 0;
	
		virtual bool patch_mem_nop(uintptr_t offset, size_t size) const noexcept;
		virtual bool patch_mem(uintptr_t offset, uint8_t* buffer, size_t size) const noexcept;
		virtual bool patch_mem(uintptr_t offset, initializer_list<uint8_t> list) const noexcept;
		virtual uintptr_t patch_iat(HMODULE current_module, const char* import_module_name,
			const char* func_name, uintptr_t func) const noexcept;
		virtual uintptr_t patch_iat_delay(HMODULE current_module, const char* import_module_name,
			const char* func_name, uintptr_t func) const noexcept;
		virtual uintptr_t patch_vtable_func(uintptr_t target, uintptr_t func, uint32_t index) const noexcept;
		virtual uintptr_t detour_jump(uintptr_t target, uintptr_t func) const noexcept;
		virtual uintptr_t detour_call(uintptr_t target, uintptr_t func) const noexcept;
	private:
		bool start_impl() const;
	};
}