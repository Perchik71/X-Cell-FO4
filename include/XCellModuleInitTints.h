// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleInitTints : public Module
	{
		REL::PatchNop useless_code_remove[4];
		REL::Patch check_function;
		REL::Patch skips_remove_tints;		
	public:
		static constexpr auto SourceName = "Module InitTints";

		ModuleInitTints(void* Context);
		virtual ~ModuleInitTints() = default;

		ModuleInitTints(const ModuleInitTints&) = delete;
		ModuleInitTints& operator=(const ModuleInitTints&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}