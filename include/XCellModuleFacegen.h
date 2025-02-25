// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleFacegen : public Module
	{
	public:
		static constexpr auto SourceName = "Module Facegen";

		ModuleFacegen(void* Context);
		virtual ~ModuleFacegen() = default;

		ModuleFacegen(const ModuleFacegen&) = delete;
		ModuleFacegen& operator=(const ModuleFacegen&) = delete;

		virtual HRESULT Listener();
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}