// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class XCellModuleGreyMovies : public Module
	{
		REL::Patch _fixes;
		REL::DetourCall _function;
	public:
		static constexpr auto SourceName = "Module GreyMovies";

		XCellModuleGreyMovies(void* Context);
		virtual ~XCellModuleGreyMovies() = default;

		XCellModuleGreyMovies(const XCellModuleGreyMovies&) = delete;
		XCellModuleGreyMovies& operator=(const XCellModuleGreyMovies&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}