// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellSettings.h"

namespace XCell
{
	// All threads have priority above idle, process above normal, for adequate latency,
	// prohibition on changing processor cores.
	extern std::shared_ptr<Setting> CVarThreads;
	// Replaced memory manager and scrap heap, also functions for working with memory.
	// It is required to remove mods that also do this or disable them in the settings of the mods themselves.
	extern std::shared_ptr<Setting> CVarMemory;
	// Enables facegen support and removes freezes.
	// If you find a character without a head, then he does not have a facegen, generate a face.
	// Creation Kit->Select npc's -> ctrl + f4.
	extern std::shared_ptr<Setting> CVarFacegen;
	// - Replacing FindFirstNextA, FindFirstNextW with a more optimized function FindFirstFileExA, FindFirstFileExW.
	// - Use OS file cache for less disk access.
	extern std::shared_ptr<Setting> CVarIO;
	// - Replace old zlib decompression code with optimized libdeflate.
	//   https://github.com/ebiggers/libdeflate
	extern std::shared_ptr<Setting> CVarLibDeflate;
	// - Replacing functions WritePrivateProfileStringA, GetPrivateProfileStringA, GetPrivateProfileIntA
	//   They are outdated and constantly open and parsing the ini file.Complements Buffout 4, Buffout 4 NG.
	//   Incompatible with the mod https://www.nexusmods.com/fallout4/mods/33947 PrivateProfileRedirector.
	//   If that mod is installed, it needs to be disabled.
	extern std::shared_ptr<Setting> CVarProfile;
	// Black loading screen.
	extern std::shared_ptr<Setting> CVarLoadScreen;
	// Increase FPS due to scaling, requires Win 8.1 and newer, and a graphics card with hardware scaling support.
	extern std::shared_ptr<Setting> CVarUpscaler;
	// Fixes a bug where movies that don't define "BackgroundAlpha" on their movie root could load with a grey background.
	extern std::shared_ptr<Setting> CVarGreyMovies;


	extern std::shared_ptr<Setting> CVarDropItem;
	// Fixes a crash when allocating the location for a package.
	extern std::shared_ptr<Setting> CVarPackageAllocateLocation;
	// Warns when a call to CreateTexture2D fails.
	extern std::shared_ptr<Setting> CVarWarningCreateTexture2D;
	// BSScaleformSysMemMapper was separated from the main x - cell memory manager.This option indicates the page size in KB,
	// which is allocated at once, if necessary, to allocate a new page, the vanilla size is 64 KB.
	// One call takes more than 1K processor cycles and turns to 0 ring, thereby stopping all threads, causing a freeze.
	// The larger this number, the fewer calls, however, memory consumption will increase.
	// Limit 2Mb(2048) installed programmatically, number must be a multiple of 8.
	extern std::shared_ptr<Setting> CVarScaleformPageSize;
	// BSScaleformSysMemMapper was separated from the main x - cell memory manager.This option indicates the heap size in MB,
	// which is reserved at once.This is all the available memory, if the memory runs out you will get a CTD.The vanilla size is 128 MB.
	// Memory is reserved, not allocated, thus there is no initial increase in memory consumption.
	// Limit 2Gb(2048) installed programmatically, number must be a multiple of 8. If you don't have even that much memory, 
	// it's worth thinking about your MCM menu.
	extern std::shared_ptr<Setting> CVarScaleformHeapSize;
	// Replaces the old 12 redistributable with a 22 one.Made into a separate option, as I'm tired of fake reports.
	// If this option is enabled, the reports will include an X-Cell in case of errors in your mods related to copying or comparing memory.
	extern std::shared_ptr<Setting> CVarUseNewRedistributable;
	// Create file "<FALLOUT4_DIR>\\Data\\F4SE\\Plugins\\rtti-x-cell.txt" with rtti info.
	extern std::shared_ptr<Setting> CVarOutputRTTI;
	// Activate a prompt for the system that you need to use a cache with random access, otherwise it will be sequential (Need bIO patch).
	extern std::shared_ptr<Setting> CVarUseIORandomAccess;
	// Scaling in for the game screen. Range: [0.5, 1]
	extern std::shared_ptr<Setting> CVarDisplayScale;
	// Do not use the original TAA, which causes slight ripples.
	extern std::shared_ptr<Setting> CVarNoUseTAA;
	// Removes the block on loading NPCs tints of the Fallout4.esm file, as well as for NPCs with set the IsChargenPresent flag.
	extern std::shared_ptr<Setting> CVarInitTints;
	// Fixes bugs when toggling references with LOD causing LOD to briefly enable and disable by removing the "Has Distant LOD" and "Visible When Distant" flag checks: https://www.youtube.com/watch?v=hgMm9Z8lHfU.
	extern std::shared_ptr<Setting> CVarLODDistance;
	// Debugging messages about the presence of facegen in the NPC in console and log (Need bFacegen patch).
	extern std::shared_ptr<Setting> CVarDbgFacegenOutput;	

	// ---

	extern std::shared_ptr<Setting> CVarLodMipBias;
	extern std::shared_ptr<Setting> CVarMaxAnisotropy;
	extern std::shared_ptr<Setting> CVarTAA;
}