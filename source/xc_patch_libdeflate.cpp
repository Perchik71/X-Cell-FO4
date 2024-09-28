// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <limits.h>
#include <libdeflate.h>
#include <xc_patch_libdeflate.h>
#include <xc_assertion.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	const char* patch_libdeflate::get_name() const noexcept
	{
		return "libdeflate";
	}

	bool patch_libdeflate::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool patch_libdeflate::run() const
	{
		auto base = GetModuleHandleA(NULL);

		//
		// libdeflate optimizations:
		//
		// - Replace old zlib decompression code with optimized libdeflate.

		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// 163
			detour_call((g_plugin->get_base() + 0x13267D), (uintptr_t)&impl_inflate_init);
			detour_call((g_plugin->get_base() + 0x1326AF), (uintptr_t)&impl_inflate);
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// 984
			detour_call((g_plugin->get_base() + 0x2A5352), (uintptr_t)&impl_inflate_init);
			detour_call((g_plugin->get_base() + 0x2A5384), (uintptr_t)&impl_inflate);
		}
		else
		{
			_ERROR("The patch \"%s\" has not been installed, as the mod does not know the game", get_name());
			return false;
		}

		return true;
	}

	int patch_libdeflate::impl_inflate_init(z_stream_s* stream, const char* version, int mode)
	{
		// Force inflateEnd to error out and skip frees
		stream->state = nullptr;

		return 0;
	}

	int patch_libdeflate::impl_inflate(z_stream_s* stream, int flush)
	{
		size_t outBytes = 0;
		libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();

		libdeflate_result result = libdeflate_zlib_decompress(decompressor, stream->next_in, stream->avail_in,
			stream->next_out, stream->avail_out, &outBytes);
		libdeflate_free_decompressor(decompressor);

		if (result == LIBDEFLATE_SUCCESS)
		{
			_xc_assert(outBytes < numeric_limits<uint32_t>::max());

			stream->total_in = stream->avail_in;
			stream->total_out = (uint32_t)outBytes;

			return 1;
		}

		if (result == LIBDEFLATE_INSUFFICIENT_SPACE)
			return -5;

		return -2;
	}
}