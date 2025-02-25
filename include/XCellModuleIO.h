// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleIO : public Module
	{
		REL::DetourIAT _functions[4];
	public:
		static constexpr auto SourceName = "Module IO";

		ModuleIO(void* Context);
		virtual ~ModuleIO() = default;

		ModuleIO(const ModuleIO&) = delete;
		ModuleIO& operator=(const ModuleIO&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}