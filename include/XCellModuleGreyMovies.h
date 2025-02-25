// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// XCell
#include "XCellModule.h"
#include "XCellRelocator.h"

namespace XCell
{
	class ModuleGreyMovies : public Module
	{
		REL::Patch _fixes;
		REL::DetourCall _function;
	public:
		static constexpr auto SourceName = "Module GreyMovies";

		ModuleGreyMovies(void* Context);
		virtual ~ModuleGreyMovies() = default;

		ModuleGreyMovies(const ModuleGreyMovies&) = delete;
		ModuleGreyMovies& operator=(const ModuleGreyMovies&) = delete;
	protected:
		virtual HRESULT InstallImpl();
		virtual HRESULT ShutdownImpl();
	};
}