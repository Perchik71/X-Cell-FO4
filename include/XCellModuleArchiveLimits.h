// Copyright © 2026 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleArchiveLimits : public Module
	{
	public:
		static constexpr auto SourceName = "Module ArchiveLimits";

		ModuleArchiveLimits(void* Context);
		virtual ~ModuleArchiveLimits() = default;

		ModuleArchiveLimits(const ModuleArchiveLimits&) = delete;
		ModuleArchiveLimits& operator=(const ModuleArchiveLimits&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}