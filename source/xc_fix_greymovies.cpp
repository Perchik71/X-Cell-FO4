// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/ScaleformValue.h>
#include <xc_fix_greymovies.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	void fix_greymovies::setbgalpha(GFxMovieView* self, float)
	{
		GFxValue alpha;

		if (!self->movieRoot->GetVariable(std::addressof(alpha), "BackgroundAlpha"))
			alpha.SetNumber(0.0f);

		// set alpha
		thisVirtualCall<void>(0x108, self, alpha.GetNumber());
	}

	const char* fix_greymovies::get_name() const noexcept
	{
		return "greymovies";
	}

	bool fix_greymovies::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool fix_greymovies::run() const
	{
#ifdef FO4NG2
		// only NG
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			auto addr = g_plugin->get_base() + 0x19FFE89;
			
			// mov rcx, rbx
			// test al, al
			// je fallout4.7FF7DD38FEAF
			// movsd xmm1, qword ptr ss:[rsp+0x40]
			// cvtpd2ps xmm1, xmm1
			// minss xmm1, dword ptr ds:[0x7FF7DDC03D00]
			// maxss xmm1, xmm7
			// jmp fallout4.7FF7DD38FEB7
			// movss xmm1, dword ptr ss:[rbp+0x150]
			patch_mem(addr, { 0x48, 0x8B, 0xCB, 0x84, 0xC0, 0x74, 0x18, 0xF2, 0x0F, 0x10, 0x4C, 0x24, 0x40, 0x66, 0x0F, 0x5A, 0xC9, 0xF3, 
				0x0F, 0x5D, 0x0D, 0x5E, 0x3E, 0x87, 0x00, 0xF3, 0x0F, 0x5F, 0xCF, 0xEB, 0x08, 0xF3, 0x0F, 0x10, 0x8D, 0x50, 0x01, 0x00, 0x00,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
			detour_call(addr + 0x27, (uintptr_t)&setbgalpha);
		}
		else
		{
			_ERROR("The fix \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
#else
		return false;
#endif // FO4NG2
	}
}