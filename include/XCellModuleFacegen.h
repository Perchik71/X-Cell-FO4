// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleFacegen : public Module
	{
	public:
		static constexpr auto SourceName = "Module Facegen";

		XCellModuleFacegen(void* Context);
		virtual ~XCellModuleFacegen() = default;

		XCellModuleFacegen(const XCellModuleFacegen&) = delete;
		XCellModuleFacegen& operator=(const XCellModuleFacegen&) = delete;

		virtual HRESULT Listener();
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}