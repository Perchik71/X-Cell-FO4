// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleLibDeflate : public Module
	{
		REL::DetourCall _functions[2];
	public:
		static constexpr auto SourceName = "Module LibDeflate";

		ModuleLibDeflate(void* Context);
		virtual ~ModuleLibDeflate() = default;

		ModuleLibDeflate(const ModuleLibDeflate&) = delete;
		ModuleLibDeflate& operator=(const ModuleLibDeflate&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}