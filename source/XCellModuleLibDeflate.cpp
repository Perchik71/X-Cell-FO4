// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <libdeflate.h>

#include "XCellTableID.h"
#include "XCellModuleLibDeflate.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

namespace XCell
{
	struct z_stream_s
	{
		const void* next_in;
		uint32_t avail_in;
		uint32_t total_in;
		void* next_out;
		uint32_t avail_out;
		uint32_t total_out;
		const char* msg;
		struct internal_state* state;
	};

	static int __stdcall HKInflateInit(z_stream_s* stream, const char* version, int mode)
	{
		// Force inflateEnd to error out and skip frees
		stream->state = nullptr;

		return 0;
	}

	static int __stdcall HKInflate(z_stream_s* stream, int flush)
	{
		size_t outBytes = 0;
		libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();

		libdeflate_result result = libdeflate_zlib_decompress(decompressor, stream->next_in, stream->avail_in,
			stream->next_out, stream->avail_out, &outBytes);
		libdeflate_free_decompressor(decompressor);

		if (result == LIBDEFLATE_SUCCESS)
		{
			XCAssert(outBytes < numeric_limits<uint32_t>::max());

			stream->total_in = stream->avail_in;
			stream->total_out = (uint32_t)outBytes;

			return 1;
		}

		if (result == LIBDEFLATE_INSUFFICIENT_SPACE)
			return -5;

		return -2;
	}

	XCellModuleLibDeflate::XCellModuleLibDeflate(void* Context) :
		Module(Context, SourceName, CVarLibDeflate)
	{
		//
		// libdeflate optimizations:
		//
		// - Replace old zlib decompression code with optimized libdeflate.

		_functions[0].Install(REL::ID(160), (UInt64)&HKInflateInit);
		_functions[1].Install(REL::ID(165), (UInt64)&HKInflate);
	}

	HRESULT XCellModuleLibDeflate::InstallImpl()
	{
		//
		// libdeflate optimizations:
		//
		// - Replace old zlib decompression code with optimized libdeflate.

		_functions[0].Enable();
		_functions[1].Enable();

		return S_OK;
	}

	HRESULT XCellModuleLibDeflate::ShutdownImpl()
	{
		// Returned

		_functions[0].Disable();
		_functions[1].Disable();

		return S_OK;
	}
}