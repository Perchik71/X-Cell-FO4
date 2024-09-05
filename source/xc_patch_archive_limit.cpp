// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <vmm.h>
#include <xc_patch_archive_limit.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	uintptr_t append_to_archive_1_orig = 0;
	uintptr_t memcpy_hash_from_archive_table_orig = 0;
	
	struct __declspec(align(4)) archive_hash
	{
		uint32_t hash;
		char ext[4];
		uint32_t pad;
	};

	constexpr static uint32_t max_limit		= 0x1000;			// 4096
	constexpr static uint32_t max_limit_v2	= max_limit * 2;	// 4096 * 2

	struct tree_db_general_t
	{
		char data_1[0x30];
		void* array_1[max_limit];
		void* array_2[max_limit];
		archive_hash array_3[max_limit];
		char data_2[0x300];
	};

	tree_db_general_t g_tree_db_general;

	const char* patch_archive_limit::get_name() const noexcept
	{
		return "archive_limit";
	}

	bool patch_archive_limit::game_data_ready_handler() const noexcept
	{
		return true;
	}

	bool patch_archive_limit::run() const
	{
		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			//// STEP 0

			{
				// Rewrites the code to work with distance further than 2GB.

				auto offset_op = g_plugin->get_base() + 0x1B7036C;

				patch_mem(offset_op, { 0x75, 0x16, 0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0xE8, 0x83, 0x45, 0x00, 0x00, 0x48, 0x89, 0x05, 0x64, 0x5C, 0x09, 0x04, 0x48, 0x83, 0xC4,
					0x28, 0xC3 });

				auto ptr = (uint8_t*)&g_tree_db_general;
				patch_mem(offset_op + 4, (uint8_t*)&ptr, 8);
				_MESSAGE("ARCHIVE_LIMIT: new db address %p.", (uintptr_t)&g_tree_db_general);
			}

			auto max_limit_hash = max_limit - 1;
			patch_mem(g_plugin->get_base() + 0x1B7470A, (uint8_t*)&max_limit_v2, 4);
			patch_mem(g_plugin->get_base() + 0x1B74779, (uint8_t*)&max_limit_hash, 4);

			auto offset = (max_limit * sizeof(void*)) + 0x30;
			patch_mem(g_plugin->get_base() + 0x1B7474C, (uint8_t*)&offset, 4);
			offset = (max_limit * sizeof(void*) * 2) + 0x30;
			patch_mem(g_plugin->get_base() + 0x1B74780, (uint8_t*)&offset, 4);

			//// STEP 1

			{
				// Correcting all offsets in function initialize
				{
					offset = g_plugin->get_base() + 0x1B747A2;
					scope_relocate_al lock((LPVOID)(g_plugin->get_base() + 0x1B746E0), 0x147);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x2D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x16;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 6;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B74946;
					scope_relocate_al lock((LPVOID)(g_plugin->get_base() + 0x1B74900), 0x23A);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x4E;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x52;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 7;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x13;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x19;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				//// STEP 2

				// Correcting all offsets in function stuffs

				{
					offset = g_plugin->get_base() + 0x1B756F0;
					scope_relocate_al lock((LPVOID)(offset), 0x128);

					offset += 0x30;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 15;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x16;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x59;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B721A0;
					scope_relocate_al lock((LPVOID)(offset), 0x3D8);

					offset += 0x72;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x16;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x1D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x146;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x4E;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x22;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0xC4;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x13;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x77;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B75A8B;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B76B4A;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B77A26;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B77B83;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B77BA2;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B77D72;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7827A;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B78B0F;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B79283;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B79293;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7A2FA;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7A572;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71F94;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}


				{
					offset = g_plugin->get_base() + 0x1B7162E;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7168E;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71726;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71776;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7277E;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x5D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B714CD;
					scope_relocate_al lock((LPVOID)(offset), 0x200);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x20;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x20;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x20;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x30;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x20;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x20;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71EE0;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71F21;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71E4D;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x19;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71DAD;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x19;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B71A87;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x18;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7195B;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x18;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7184B;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x17;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7A8BC;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x7D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7201A;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x1C;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B6F3C7;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x36;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7C233;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7C243;
					scope_relocate_al lock((LPVOID)(offset), 0x4);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}
			}

			//// STEP 3

			// Correcting all offsets in function stuffs (get / search / set)

			{
				{
					offset = g_plugin->get_base() + 0x1B75600;
					scope_relocate_al lock((LPVOID)(offset), 0xDE);

					offset += 0x16;
					*((uint32_t*)offset) = max_limit_hash;
					offset += 10;
					*((uint32_t*)offset) = (max_limit * sizeof(void*) * 2) + 0x30;
					offset += 0x4D;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
				}

				{
					offset = g_plugin->get_base() + 0x1B71BBE;
					scope_relocate_al lock((LPVOID)(offset), 0x25);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x12;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7371A;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x8C;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}
				
				{
					offset = g_plugin->get_base() + 0x1B6F6C0;
					scope_relocate_al lock((LPVOID)(offset), 0x1000);

					offset += 0x118;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x2C;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x1E;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x497;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x45C;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0xE4;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B72A50;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					offset += 0x1A;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x43;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x4D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x23;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B73876;
					scope_relocate_al lock((LPVOID)(offset), 0x100);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x68;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B72895;
					scope_relocate_al lock((LPVOID)(offset), 0x70);

					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x1A;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0xA;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
				}

				{
					offset = g_plugin->get_base() + 0x1B78A70;
					scope_relocate_al lock((LPVOID)(offset), 0x35);

					offset += 0x9;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x11;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
				}

				{
					offset = g_plugin->get_base() + 0x1B709D0;
					scope_relocate_al lock((LPVOID)(offset), 0x2A3);

					offset += 0x4F;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x16;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0xA;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
					offset += 0x5A;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B767F0;
					scope_relocate_al lock((LPVOID)(offset), 0xF6);

					offset += 0x31;
					*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*)) + 0x30;
					offset += 0x13;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x17;
					*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x38;
					offset += 0x14;
					*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x30;
					offset += 0x6;
					*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x30;
					offset += 0x16;
					*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x34;
					offset += 0x55;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B78AB0;
					scope_relocate_al lock((LPVOID)(offset), 0x32);

					offset += 0x9;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}

				{
					offset = g_plugin->get_base() + 0x1B7A0C0;
					scope_relocate_al lock((LPVOID)(offset), 0x1E5);

					offset += 0x16;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x3E;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x8C;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x5A;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x28;
					offset += 0xD;
					*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x28;
					offset += 0x33;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
					offset += 0x1D;
					*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				}
			}

			////////////////////////////////

			// supported 8 version archive
			patch_mem_nop(g_plugin->get_base() + 0x1B6FA9F, { 0x8 });

#if 0
			detour_call(g_plugin->get_base() + 0x1B76AC9, (uintptr_t)&impl_memcpy_hash_from_archive_table);
			memcpy_hash_from_archive_table_orig = g_plugin->get_base() + 0x1B78970;

			detour_call(g_plugin->get_base() + 0x1B76AF7, (uintptr_t)&impl_set_index_archive_to_hash_og);
#endif
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			//// STEP 0

			{
				// Rewrites the code to work with distance further than 2GB.

				auto offset_op = g_plugin->get_base() + 0x1581C5C;

				patch_mem(offset_op, { 0x75, 0x16, 0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0xE8, 0x03, 0x37, 0x00, 0x00, 0x48, 0x89, 0x05, 0x74, 0xD7, 0xC0, 0x01, 0x48, 0x83, 0xC4,
					0x28, 0xC3 });

				auto ptr = (uint8_t*)&g_tree_db_general;
				patch_mem(offset_op + 4, (uint8_t*)&ptr, 8);
				_MESSAGE("ARCHIVE_LIMIT: new db address %p.", (uintptr_t)&g_tree_db_general);
			}

			patch_mem(g_plugin->get_base() + 0x15853A6, (uint8_t*)&max_limit_v2, 4);
			patch_mem(g_plugin->get_base() + 0x15853F9, (uint8_t*)&max_limit_v2, 4);
			patch_mem(g_plugin->get_base() + 0x15854A1, (uint8_t*)&max_limit, 4);

			auto offset = (max_limit * sizeof(void*)) + 0x30;
			patch_mem(g_plugin->get_base() + 0x158546C, (uint8_t*)&offset, 4);
			offset = (max_limit * sizeof(void*) * 2) + 0x30;
			patch_mem(g_plugin->get_base() + 0x158549C, (uint8_t*)&offset, 4);

			//// STEP 1

			{
				offset = g_plugin->get_base() + 0x15854C4;
				scope_relocate_al lock((LPVOID)(g_plugin->get_base() + 0x1585370), 0x3A1);

				// Correcting all offsets in function initialize
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 10;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 27;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 27;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 72;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 79;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 19;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 22;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			//// STEP 2

			// Correcting all offsets in function stuffs

			{
				offset = g_plugin->get_base() + 0x1580FB5;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1580FED;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15812B0;
				scope_relocate_al lock((LPVOID)(offset), 0x8D8);

				offset += 0x128;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 14;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x3B;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x3E;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x22;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 733;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 700;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 221;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15832FD;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x158331D;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x158333D;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x158335D;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x158338D;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15833AD;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15833CD;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1584C00;
				scope_relocate_al lock((LPVOID)(offset), 0x100);

				offset += 0x60;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x69;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1584D10;
				scope_relocate_al lock((LPVOID)(offset), 0x143);

				offset += 0x66;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x7D;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1581C80;
				scope_relocate_al lock((LPVOID)(offset), 0xE6);

				offset += 0x27;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x27;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x16;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 12;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15834BE;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15835F6;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583646;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583670;
				scope_relocate_al lock((LPVOID)(offset), 0x113);

				offset += 0xAB;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1E;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583790;
				scope_relocate_al lock((LPVOID)(offset), 0x104);

				offset += 0x9B;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1F;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15838A0;
				scope_relocate_al lock((LPVOID)(offset), 0x11E);

				offset += 0xB7;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1F;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583A8E;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x158345E;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583F82;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583EB0;
				scope_relocate_al lock((LPVOID)(offset), 0x86);

				offset += 0x1D;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x19;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583E3D;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583DCA;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583D93;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583CE0;
				scope_relocate_al lock((LPVOID)(offset), 0x8D);

				offset += 0x1D;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x19;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1583C40;
				scope_relocate_al lock((LPVOID)(offset), 0x8D);

				offset += 0x1D;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x19;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1588F32;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1587D41;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x15863C0;
				scope_relocate_al lock((LPVOID)(offset), 0x20E);

				offset += 0x13F;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x21;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1589D3E;
				scope_relocate_al lock((LPVOID)(offset), 0x4);

				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			//// STEP 3

			// Correcting all offsets in function stuffs (get / search / set)

			{
				offset = g_plugin->get_base() + 0x15862C0;
				scope_relocate_al lock((LPVOID)(offset), 0xF6);

				offset += 0x31;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*)) + 0x30;
				offset += 0x13;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x17;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x38;
				offset += 0x14;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x30;
				offset += 0x6;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x30;
				offset += 0x16;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*) * 2) + 0x34;
				offset += 0x55;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1585B40;
				scope_relocate_al lock((LPVOID)(offset), 0x10E);

				offset += 0x16;
				*((uint32_t*)offset) = max_limit;
				offset += 7;
				*((uint32_t*)offset) = (max_limit * sizeof(void*) * 2) + 0x30;
				offset += 0x61;
				*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
			}

			{
				offset = g_plugin->get_base() + 0x1584670;
				scope_relocate_al lock((LPVOID)(offset), 0x22A);

				offset += 0x95;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1C;
				*((uint32_t*)offset) = (max_limit * sizeof(void*)) + 0x30;
			}

			{
				offset = g_plugin->get_base() + 0x1584560;
				scope_relocate_al lock((LPVOID)(offset), 0x107);

				offset += 0x2E;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x16;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1C;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x8B;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1584030;
				scope_relocate_al lock((LPVOID)(offset), 0x478);

				offset += 0x72;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x16;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1D;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x11E;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x37;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0xB5;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x1A;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x74;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			{
				offset = g_plugin->get_base() + 0x1582590;
				scope_relocate_al lock((LPVOID)(offset), 0x2A3);

				offset += 0x4F;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0x16;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
				offset += 0xA;
				*((uint32_t*)offset) = (uint32_t)(max_limit * sizeof(void*)) + 0x30;
				offset += 0x5A;
				*((uint32_t*)offset) = *((uint32_t*)offset) - 0x1C00 + 0x1C000;
			}

			////////////////////////////////

			detour_call(g_plugin->get_base() + 0x158646F, (uintptr_t)&impl_memcpy_hash_from_archive_table);
			memcpy_hash_from_archive_table_orig = g_plugin->get_base() + 0x1587BA0;

			detour_call(g_plugin->get_base() + 0x15864B5, (uintptr_t)&impl_set_index_archive_to_hash_ng);

			offset = g_plugin->get_base() + 1587095;
			// Remove useless stuff.	
			patch_mem_nop(offset, 0x16);
			// mov eax, dword ptr ds:[rsi+0xC]
			// mov dword ptr ds:[rdi+0xC], eax
			patch_mem(offset, { 0x8B, 0x46, 0x0C, 0x89, 0x47, 0x0C });
		}

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

	void patch_archive_limit::impl_set_index_archive_to_hash_og()
	{
		// It is necessary to get the stack of the calling function.
		auto rsp = (uintptr_t)_AddressOfReturnAddress() + 8;
		// Set archive index from stack
		*((uint16_t*)(rsp + 0x4C)) = *((uint16_t*)(rsp + 0x250));
	}

	void patch_archive_limit::impl_set_index_archive_to_hash_ng()
	{
		// It is necessary to get the stack of the calling function.
		auto rsp = (uintptr_t)_AddressOfReturnAddress() + 8;
		// Set archive index from stack
		*((uint16_t*)(rsp + 0x3C)) = *((uint16_t*)(rsp + 0x1E8));
	}
}