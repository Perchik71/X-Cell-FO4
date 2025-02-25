// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleWarningCreateTexture2D.h"

#include <comdef.h>

#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"
#include "XCellAssertion.h"

namespace XCell
{
	UInt64 _OldCreateTexture2D = 0;

	static HRESULT STDMETHODCALLTYPE HKCreateTexture2D(ID3D11Device* Device, const D3D11_TEXTURE2D_DESC* pDesc,
		const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
	{
		auto hr = XCFastCall<HRESULT>(_OldCreateTexture2D, Device, pDesc, pInitialData, ppTexture2D);
		XCAssertWithFormattedMessage(hr == S_OK, "A call to ID3D11Device::CreateTexture2D failed with error code 0x%08X.\n"
			"Message: %s\nThis will crash the game.", _com_error(hr).ErrorMessage());
		return hr;
	}

	ModuleWarningCreateTexture2D::ModuleWarningCreateTexture2D(void* Context) :
		Module(Context, SourceName, CVarWarningCreateTexture2D, XCELL_MODULE_QUERY_DIRECTX_INIT),
		_Device(nullptr)
	{
		InitializeDirectXLinker.OnListener = (EventInitializeDirectXSourceLink::EventFunctionType)(&ModuleWarningCreateTexture2D::DXListener);
	}

	HRESULT ModuleWarningCreateTexture2D::DXListener(HWND WindowHandle, ID3D11Device* Device, ID3D11DeviceContext* Context,
		IDXGISwapChain* SwapChain)
	{
		_Device = Device;
		if (_Device)
			_OldCreateTexture2D = REL::Impl::DetourVTable(*((UInt64*)Device), (UInt64)&HKCreateTexture2D, 5);
		return _OldCreateTexture2D ? S_OK : E_FAIL;
	}

	HRESULT ModuleWarningCreateTexture2D::InstallImpl()
	{
		return S_OK;
	}

	HRESULT ModuleWarningCreateTexture2D::ShutdownImpl()
	{
		if (_Device) REL::Impl::DetourVTable(*((UInt64*)_Device), _OldCreateTexture2D, 5);

		return S_OK;
	}
}