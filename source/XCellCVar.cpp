// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellCVar.h"

namespace XCell
{
	std::shared_ptr<Setting> CVarThreads = std::make_shared<Setting>("bThreads:Patches", true);
	std::shared_ptr<Setting> CVarMemory = std::make_shared<Setting>("bMemory:Patches", true);
	std::shared_ptr<Setting> CVarFacegen = std::make_shared<Setting>("bFacegen:Patches", true);
	std::shared_ptr<Setting> CVarIO = std::make_shared<Setting>("bIO:Patches", true);
	std::shared_ptr<Setting> CVarLibDeflate = std::make_shared<Setting>("bLibDeflate:Patches", true);
	std::shared_ptr<Setting> CVarProfile = std::make_shared<Setting>("bProfile:Patches", true);
	std::shared_ptr<Setting> CVarLoadScreen = std::make_shared<Setting>("bLoadScreen:Patches", false);
	std::shared_ptr<Setting> CVarUpscaler = std::make_shared<Setting>("bUpscaler:Patches", false);

	std::shared_ptr<Setting> CVarInitTints = std::make_shared<Setting>("bInitTints:Fixes", true);
	std::shared_ptr<Setting> CVarLODDistance = std::make_shared<Setting>("bLODDistance:Fixes", true);

	std::shared_ptr<Setting> CVarDropItem = std::make_shared<Setting>("bDropItem:NGPatches", true);

	std::shared_ptr<Setting> CVarGreyMovies = std::make_shared<Setting>("bGreyMovies:NGFixes", true);
	std::shared_ptr<Setting> CVarPackageAllocateLocation = std::make_shared<Setting>("bPackageAllocateLocation:NGFixes", true);
	std::shared_ptr<Setting> CVarWarningCreateTexture2D = std::make_shared<Setting>("bWarningCreateTexture2D:NGFixes", false);

	std::shared_ptr<Setting> CVarScaleformPageSize = std::make_shared<Setting>("uScaleformPageSize:Additional", (uint32_t)256ul);
	std::shared_ptr<Setting> CVarScaleformHeapSize = std::make_shared<Setting>("uScaleformHeapSize:Additional", (uint32_t)512ul);
	std::shared_ptr<Setting> CVarUseNewRedistributable = std::make_shared<Setting>("bUseNewRedistributable:Additional", false);
	std::shared_ptr<Setting> CVarOutputRTTI = std::make_shared<Setting>("bOutputRTTI:Additional", false);
	std::shared_ptr<Setting> CVarUseIORandomAccess = std::make_shared<Setting>("bUseIORandomAccess:Additional", false);
	std::shared_ptr<Setting> CVarDbgFacegenOutput = std::make_shared<Setting>("bDbgFacegenOutput:Additional", false);

	std::shared_ptr<Setting> CVarLodMipBias = std::make_shared<Setting>("fLodMipBias:Graphics", 0.0f);
	std::shared_ptr<Setting> CVarMaxAnisotropy = std::make_shared<Setting>("uMaxAnisotropy:Graphics", 0);

	std::shared_ptr<Setting> CVarDisplayScale = std::make_shared<Setting>("fDisplayScale:PostProccessing", 0.75f);
	std::shared_ptr<Setting> CVarNoUseTAA = std::make_shared<Setting>("bNoUseOriginalTAA:PostProccessing", false);
	std::shared_ptr<Setting> CVarTAA = std::make_shared<Setting>("bEnableTAA:PostProccessing", true);
}