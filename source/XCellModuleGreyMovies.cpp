// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <f4se/ScaleformMovie.h>
#include <f4se/ScaleformValue.h>

#include "XCellModuleGreyMovies.h"

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

namespace XCell
{
	static void __stdcall HKGfxSetBGAlpha(GFxMovieView* self, float)
	{
		GFxValue alpha;

		if (!self->movieRoot->GetVariable(std::addressof(alpha), "BackgroundAlpha"))
			alpha.SetNumber(0.0f);

		// Set alpha
		XCThisVirtualCall<void>(0x108, self, alpha.GetNumber());
	}

	XCellModuleGreyMovies::XCellModuleGreyMovies(void* Context) :
		Module(Context, SourceName, CVarGreyMovies)
	{
		if (REL::Version() == RUNTIME_VERSION_1_10_984)
		{
			// mov rcx, rbx
			// test al, al
			// je fallout4.7FF7DD38FEAF
			// movsd xmm1, qword ptr ss:[rsp+0x40]
			// cvtpd2ps xmm1, xmm1
			// minss xmm1, dword ptr ds:[0x7FF7DDC03D00]
			// maxss xmm1, xmm7
			// jmp fallout4.7FF7DD38FEB7
			// movss xmm1, dword ptr ss:[rbp+0x150]

			auto Target = REL::ID(180);
			_fixes.Install(Target, { 0x48, 0x8B, 0xCB, 0x84, 0xC0, 0x74, 0x18, 0xF2, 0x0F, 0x10, 0x4C, 0x24, 0x40, 0x66, 0x0F, 0x5A, 0xC9, 0xF3,
				0x0F, 0x5D, 0x0D, 0x5E, 0x3E, 0x87, 0x00, 0xF3, 0x0F, 0x5F, 0xCF, 0xEB, 0x08, 0xF3, 0x0F, 0x10, 0x8D, 0x50, 0x01, 0x00, 0x00,
				0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
			_function.Install(Target + 0x27, (UInt64)&HKGfxSetBGAlpha);
		}
	}

	HRESULT XCellModuleGreyMovies::InstallImpl()
	{
		_fixes.Enable();
		_function.Enable();

		return S_OK;
	}

	HRESULT XCellModuleGreyMovies::ShutdownImpl()
	{
		// Returned
		
		_fixes.Disable();
		_function.Disable();

		return S_OK;
	}
}