// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <vector>

// XCell
#include "XCellModule.h"

namespace XCell
{
	class ModuleManager
	{
		ICriticalSection _section;
		vector<shared_ptr<Module>> _objects;
	public:
		ModuleManager() = default;
		virtual ~ModuleManager();

		virtual HRESULT Add(const shared_ptr<Module>& Mod) noexcept(true);
		virtual HRESULT Remove(const shared_ptr<Module>& Mod) noexcept(true);
		virtual HRESULT Remove(const char* ModName) noexcept(true);
		virtual HRESULT Clear() noexcept(true);
		virtual UInt64 Size() const noexcept(true);
		virtual shared_ptr<Module> At(UInt64 Index) const noexcept(true);
		virtual shared_ptr<Module> FindByName(const char* ModName) const noexcept(true);
		virtual void Lock() const noexcept(true);
		virtual void Unlock() const noexcept(true);

		inline shared_ptr<Module> operator[](UInt64 Index) const noexcept(true) { return At(Index); }

		ModuleManager(const ModuleManager&) = delete;
		ModuleManager& operator=(const ModuleManager&) = delete;
	};
}
