// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleManager.h"

namespace XCell
{
	ModuleManager::~ModuleManager()
	{
		Clear();
	}

	HRESULT ModuleManager::Add(const shared_ptr<Module>& Mod) noexcept(true)
	{
		IScopedCriticalSection Lock(&_section);

		if (_objects.size() == _objects.max_size())
			return E_FAIL;

		for (auto It = _objects.begin(); It != _objects.end(); It++)
		{
			if (!_stricmp((*It)->Name.c_str(), Mod->Name.c_str()))
			{
				_WARNING("Module \"%s\" has already exists", Mod->Name.c_str());
				return E_FAIL;
			}
		}

		_objects.push_back(Mod);

		return S_OK;
	}

	HRESULT ModuleManager::Remove(const shared_ptr<Module>& Mod) noexcept(true)
	{
		IScopedCriticalSection Lock(&_section);

		for (auto It = _objects.begin(); It != _objects.end(); It++)
		{
			if ((*It).get() == Mod.get())
			{
				_objects.erase(It);
				return S_OK;
			}
		}

		return E_FAIL;
	}

	HRESULT ModuleManager::Remove(const char* ModName) noexcept(true)
	{
		IScopedCriticalSection Lock(&_section);

		for (auto It = _objects.begin(); It != _objects.end(); It++)
		{
			if (!_stricmp((*It)->Name.c_str(), ModName))
			{
				_objects.erase(It);
				return S_OK;
			}
		}

		return E_FAIL;
	}

	HRESULT ModuleManager::Clear() noexcept(true)
	{
		IScopedCriticalSection Lock(&_section);
		_objects.clear();
		return S_OK;
	}

	UInt64 ModuleManager::Size() const noexcept(true)
	{
		IScopedCriticalSection Lock(&(const_cast<ModuleManager*>(this)->_section));
		return (UInt64)_objects.size();
	}

	shared_ptr<Module> ModuleManager::At(UInt64 Index) const noexcept(true)
	{
		IScopedCriticalSection Lock(&(const_cast<ModuleManager*>(this)->_section));
		return _objects[Index];
	}

	shared_ptr<Module> ModuleManager::FindByName(const char* ModName) const noexcept(true)
	{
		IScopedCriticalSection Lock(&(const_cast<ModuleManager*>(this)->_section));

		for (auto It = _objects.begin(); It != _objects.end(); It++)
		{
			if (!_stricmp((*It)->Name.c_str(), ModName))
				return *It;
		}

		return nullptr;
	}

	void ModuleManager::Lock() const noexcept(true)
	{
		(const_cast<ModuleManager*>(this)->_section).Enter();
	}

	void ModuleManager::Unlock() const noexcept(true)
	{
		(const_cast<ModuleManager*>(this)->_section).Leave();
	}
}