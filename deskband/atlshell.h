#ifndef __ATLSHELL_H__
#define __ATLSHELL_H__

#ifndef __cplusplus
#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atlbase.h>
#include <shlobj.h>
#include <comdef.h>

struct _ATL_TYPEMAP_ENTRY
{
	enum Type { Undefined = 0, DeskBand, VerticalExplorerBar, HorizontalExplorerBar };

	const CLSID *pclsid;
	Type type;
};

#define BEGIN_TYPE_MAP(x) static _ATL_TYPEMAP_ENTRY x[] = {
#define END_TYPE_MAP() { NULL, _ATL_TYPEMAP_ENTRY::Undefined } };
#define TYPE_ENTRY(clsid, type) { &clsid, _ATL_TYPEMAP_ENTRY::type },

class CShellModule : public CComModule
{
public:
	CShellModule() : m_pTypeMap( NULL ) {}

	HRESULT Init( _ATL_TYPEMAP_ENTRY *pdt, _ATL_OBJMAP_ENTRY *p, HINSTANCE h )
	{
		HRESULT hr = CComModule::Init( p, h );

		if( FAILED( hr ) )
			return hr;

		ATLASSERT( pdt );
		m_pTypeMap = pdt;
		return S_OK;
	}

	HRESULT RegisterServer( const CLSID *pCLSID = NULL )
	{
		HRESULT hr = CComModule::RegisterServer( FALSE, pCLSID );	// registers object

		if( SUCCEEDED( hr ) )
		{
			// register the component categories for the desk band object
			ICatRegister *pcr = NULL;

			::CoInitialize( NULL );
			hr = ::CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, 
				IID_ICatRegister, (LPVOID*)&pcr );

			if( SUCCEEDED( hr ) && pcr )
			{
				ATLASSERT( m_pTypeMap );

				for( _ATL_TYPEMAP_ENTRY *pEntry = m_pTypeMap; pEntry->pclsid; ++pEntry )
				{
					if( pCLSID && ! ::IsEqualGUID( *pCLSID, *pEntry->pclsid ) )
						continue;

					hr = pcr->RegisterClassImplCategories( *pEntry->pclsid, 1, GetCATID( pEntry->type ) );

					if( FAILED( hr ) )
						break;
				}

				pcr->Release();
			}

			::CoUninitialize();
		}

		return SUCCEEDED( hr ) ? S_OK : SELFREG_E_CLASS;
	}

	HRESULT UnregisterServer( const CLSID *pCLSID = NULL )
	{
		// unregister the component categories for the desk band object
		ICatRegister *pcr = NULL;

		::CoInitialize( NULL );
		HRESULT hr = ::CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER,
			IID_ICatRegister, (LPVOID*)&pcr );

		if( SUCCEEDED( hr ) && pcr )
		{
			ATLASSERT( m_pTypeMap );

			for( _ATL_TYPEMAP_ENTRY *pEntry = m_pTypeMap; pEntry->pclsid; ++pEntry )
			{
				if( pCLSID && ! ::IsEqualGUID( *pCLSID, *pEntry->pclsid ) )
					continue;

				hr = pcr->UnRegisterClassImplCategories( *pEntry->pclsid, 1, GetCATID( pEntry->type ) );

				if( FAILED( hr ) )
					break;
			}

			pcr->Release();
		}

		::CoUninitialize();

		if( FAILED( hr ) )
			return SELFREG_E_CLASS;

		return CComModule::UnregisterServer( FALSE, pCLSID );	// unregisters object
	}

protected:
	_ATL_TYPEMAP_ENTRY *m_pTypeMap;

private:
	CATID * GetCATID( _ATL_TYPEMAP_ENTRY::Type eType )
	{
		switch( eType )
		{
			case _ATL_TYPEMAP_ENTRY::DeskBand:
				return const_cast<CATID*>( &CATID_DeskBand );

			case _ATL_TYPEMAP_ENTRY::VerticalExplorerBar:
				return const_cast<CATID*>( &CATID_InfoBand );

			case _ATL_TYPEMAP_ENTRY::HorizontalExplorerBar:
				return const_cast<CATID*>( &CATID_CommBand );
		}

		ATLTRACE( "Undefined shell object type!\n" );
		ATLASSERT( false );
		return NULL;
	}
};

extern CShellModule _Module;
#include <atlcom.h>

struct _ATL_MENUMAP_ENTRY
{
	UINT idCmd;
	UINT uFlags;
	LPCTSTR lpItem;
	LPCTSTR lpHelpText;
	LPCTSTR lpVerb;
};

#define DECLARE_MENU_MAP(x) extern const _ATL_MENUMAP_ENTRY x[];
#define BEGIN_MENU_MAP(x) const _ATL_MENUMAP_ENTRY x[] = {
#define END_MENU_MAP() { (UINT)-1, NULL, NULL, NULL } };
#define MENU_ENTRY(id,flags,item,help,verb) { id, flags | MF_STRING, item, help, verb },
#define MENU_ENTRY_SEPARATOR() {0,MF_SEPARATOR,NULL,NULL,NULL},

template<const CLSID *pclsid,const _ATL_MENUMAP_ENTRY *pMenu = NULL>
class ATL_NO_VTABLE CDeskBand : public CComObjectRootEx<CComSingleThreadModel>,
	IObjectWithSite, IPersistStream, IDeskBand/*, IContextMenu*/
{
public:
	CDeskBand() : m_pSite( NULL ), m_hWnd( NULL ) {}

	BEGIN_COM_MAP(CDeskBand)
		COM_INTERFACE_ENTRY(IObjectWithSite)
		COM_INTERFACE_ENTRY(IPersist)
		COM_INTERFACE_ENTRY(IPersistStream)
		COM_INTERFACE_ENTRY(IOleWindow)
		COM_INTERFACE_ENTRY(IDockingWindow)
		COM_INTERFACE_ENTRY(IDeskBand)
//		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	// IObjectWithSite methods
	STDMETHOD(SetSite)( IUnknown *punkSite )
	{
		// if a site is being held, release it
		if( m_pSite )
		{
			m_pSite->Release();
			m_pSite = NULL;
		}

		// if punkSite is not NULL, a new site is being set
		if( punkSite )
		{
			// get the parent window.
			IOleWindow  *pOleWindow;
			HWND hWndParent = NULL;

			if( SUCCEEDED( punkSite->QueryInterface( IID_IOleWindow, (LPVOID*)&pOleWindow ) ) )
			{
				pOleWindow->GetWindow( &hWndParent );
				pOleWindow->Release();
			}

			if( ! hWndParent )
				return E_FAIL;

			// if the window doesn't exist yet, create it now
			if( ! m_hWnd )
			{
				// if the window class has not been registered, then do so
				WNDCLASS wc;
				LPCTSTR lpClassName = TEXT("vwBetterPagerHost");

				if( ! ::GetClassInfo( _Module.m_hInst, lpClassName, &wc ) )
				{
					::ZeroMemory( &wc, sizeof(wc) );
					wc.style			= 0;//CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
					wc.lpfnWndProc		= (WNDPROC)CDeskBand::WndProc;
					wc.cbClsExtra		= 0;
					wc.cbWndExtra		= 0;
					wc.hInstance		= _Module.m_hInst;
					wc.hIcon			= NULL;
					wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW );
					wc.hbrBackground	= (HBRUSH)::CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
					wc.lpszMenuName		= NULL;
					wc.lpszClassName	= lpClassName;

					::RegisterClass( &wc ); // if RegisterClass fails, CreateWindow below will fail
				}

				RECT rc;
				::GetClientRect( hWndParent, &rc );

				// create the window, the WndProc will set m_hWnd
				::CreateWindowEx( 0, lpClassName, NULL, WS_CHILD | WS_EX_TRANSPARENT,
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWndParent, NULL, _Module.m_hInst, (LPVOID)this );
			}

			if( ! m_hWnd )
				return E_FAIL;

			m_pSite = punkSite;	// keep the site pointer
		}

		return S_OK;
	}

	STDMETHOD(GetSite)( REFIID riid, LPVOID *ppvSite )
	{
		if( ! ppvSite )
			return E_INVALIDARG;

		*ppvSite = NULL;
		return m_pSite ? m_pSite->QueryInterface( riid, ppvSite ) : E_FAIL;
	}

	// IPersist methods
	STDMETHOD(GetClassID)( LPCLSID pClassID )
	{
		if( ! pClassID )
			return E_INVALIDARG;

		*pClassID = *pclsid;
		return S_OK;
	}

	// IPersistStream methods
	STDMETHOD(IsDirty)()
	{
		return S_FALSE;
	}

	STDMETHOD(Load)( LPSTREAM /*pStm*/ )
	{
		return S_OK;
	}

	STDMETHOD(Save)( LPSTREAM /*pStm*/, BOOL /*fClearDirty*/ )
	{
		return S_OK;
	}

	STDMETHOD(GetSizeMax)( ULARGE_INTEGER * /*pcbSize*/ )
	{
		return E_NOTIMPL;
	}

	// IOleWindow methods
	STDMETHOD(GetWindow)( HWND *phWnd )
	{
		if( ! phWnd )
			return E_INVALIDARG;

		*phWnd = m_hWnd;
		return S_OK;
	}

	STDMETHOD(ContextSensitiveHelp)( BOOL /*fEnterMode*/ )
	{
		return E_NOTIMPL;
	}

	// IDockingWindow methods
	STDMETHOD(CloseDW)( DWORD /*dwReserved*/ )
	{
		ShowDW( FALSE );

		if( ::IsWindow( m_hWnd ) )
			::DestroyWindow( m_hWnd );

		m_hWnd = NULL;
		return S_OK;
	}

	STDMETHOD(ResizeBorderDW)( LPCRECT /*prcBorder*/, IUnknown * /*punkToolbarSite*/, BOOL /*fReserved*/ )
	{
		// this method is never called for Band objects
		return E_NOTIMPL;
	}

	STDMETHOD(ShowDW)( BOOL bShow )
	{
		if( m_hWnd )
			::ShowWindow( m_hWnd, bShow ? SW_SHOW : SW_HIDE );

		return S_OK;
	}

	// IDeskBand methods
	STDMETHOD(GetBandInfo)( DWORD /*dwBandID*/, DWORD dwViewMode, DESKBANDINFO *pdbi )
	{
		if( ! pdbi )
			return E_INVALIDARG;

		if( pdbi->dwMask & DBIM_MINSIZE )
		{
			if( dwViewMode & DBIF_VIEWMODE_FLOATING )
			{
				pdbi->ptMinSize.x = 10;
				pdbi->ptMinSize.y = 10;
			}
			else
			{
				pdbi->ptMinSize.x = 10;
				pdbi->ptMinSize.y = 0;
			}
		}

		if( pdbi->dwMask & DBIM_MAXSIZE )
		{
			pdbi->ptMaxSize.x = -1;
			pdbi->ptMaxSize.y = -1;
		}

		if( pdbi->dwMask & DBIM_INTEGRAL )
		{
			pdbi->ptIntegral.x = 1;
			pdbi->ptIntegral.y = 1;
		}

		if( pdbi->dwMask & DBIM_ACTUAL )
		{
			pdbi->ptActual.x = 100;
			pdbi->ptActual.y = 0;
		}

		if( pdbi->dwMask & DBIM_TITLE )
			pdbi->dwMask &= ~DBIM_TITLE;

		if( pdbi->dwMask & DBIM_MODEFLAGS )
		{
			pdbi->dwModeFlags = DBIMF_NORMAL;
			pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
		}

		if( pdbi->dwMask & DBIM_BKCOLOR )
			pdbi->dwMask &= ~DBIM_BKCOLOR;

		return S_OK;
	}

	// IContextMenu methods
	STDMETHOD(GetCommandString)( UINT idCmd, UINT uFlags, LPUINT /*pwReserved*/, LPSTR lpszName, UINT cchMax )
	{
		if( pMenu )
		{
			for( const _ATL_MENUMAP_ENTRY *pEntry = pMenu; pEntry->idCmd != (UINT)-1; ++pEntry )
			{
				if( idCmd == pEntry->idCmd )
				{
					switch( uFlags )
					{
						case GCS_HELPTEXTA:
#ifdef UNICODE
							::WideCharToMultiByte( CP_ACP, 0, pEntry->lpHelpText, -1, lpszName, cchMax, NULL, NULL );
#else
							::lstrcpyn( lpszName, pEntry->lpHelpText, cchMax );
#endif // UNICODE
							break;

						case GCS_HELPTEXTW:
#ifdef UNICODE
							::lstrcpyn( reinterpret_cast<LPWSTR>( lpszName ), pEntry->lpHelpText, cchMax );
#else
							::MultiByteToWideChar( CP_ACP, 0, pEntry->lpHelpText, -1, reinterpret_cast<LPWSTR>( lpszName ), cchMax );
#endif // UNICODE
							break;

						case GCS_VERBA:
#ifdef UNICODE
							::WideCharToMultiByte( CP_ACP, 0, pEntry->lpVerb, -1, lpszName, cchMax, NULL, NULL );
#else
							::lstrcpyn( lpszName, pEntry->lpVerb, cchMax );
#endif // UNICODE
							break;

						case GCS_VERBW:
#ifdef UNICODE
							::lstrcpyn( reinterpret_cast<LPWSTR>( lpszName ), pEntry->lpVerb, cchMax );
#else
							::MultiByteToWideChar( CP_ACP, 0, pEntry->lpVerb, -1, reinterpret_cast<LPWSTR>( lpszName ), cchMax );
#endif // UNICODE
							break;

						case GCS_VALIDATE:
							break;

						default:
							return E_INVALIDARG;
					}

					break;
				}
			}
		}

		return NOERROR;
	}

	STDMETHOD(InvokeCommand)( LPCMINVOKECOMMANDINFO pici )
	{
		if( pMenu )
		{
			for( const _ATL_MENUMAP_ENTRY *pEntry = pMenu; pEntry->idCmd != (UINT)-1; ++pEntry )
			{
				if( LOWORD( pici->lpVerb ) == pEntry->idCmd )
				{
					OnCommand( pEntry->idCmd, NULL );
					break;
				}
			}
		}

		return NOERROR;
	}

	STDMETHOD(QueryContextMenu)( HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags )
	{
		if( pMenu )
		{
			if( ! ( uFlags & CMF_DEFAULTONLY ) )
			{
				UINT idMax = idCmdFirst;

				for( const _ATL_MENUMAP_ENTRY *pEntry = pMenu; pEntry->idCmd != (UINT)-1; ++pEntry )
				{
					if( idCmdFirst + pEntry->idCmd < idCmdLast )
					{
						if( ::InsertMenu( hmenu, indexMenu, pEntry->uFlags | MF_BYPOSITION, idCmdFirst + pEntry->idCmd,
								pEntry->lpItem ) && idCmdFirst + pEntry->idCmd >= idMax )
							idMax = idMax == idCmdFirst ? idMax + 1 : idCmdFirst + pEntry->idCmd + 1;
					}
				}

				return MAKE_HRESULT( SEVERITY_SUCCESS, 0, USHORT( idMax == idCmdFirst ? 0 : idMax - idCmdFirst ) );
			}
		}

		return MAKE_HRESULT( SEVERITY_SUCCESS, 0, USHORT( 0 ) );
	}

protected:
	virtual LRESULT OnCreate( LPCREATESTRUCT /*lpCreateStruct*/ )
	{
		return 0;
	}

	virtual LRESULT OnDestroy()
	{
		return 0;
	}

	virtual LRESULT OnPaint()
	{
		return 0;
	}

	virtual LRESULT OnCommand( WPARAM /*wParam*/, LPARAM /*lParam*/ )
	{
		return 0;
	}

	virtual LRESULT OnSize( WPARAM /*wParam*/, LPARAM /*lParam*/ )
	{
		return 0;
	}

	virtual LRESULT OnMsg( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
	{
		return ::DefWindowProc( hWnd, uMessage, wParam, lParam );
	}

	HWND m_hWnd;
	IUnknown *m_pSite;

private:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
	{
		CDeskBand *pThis = (CDeskBand*)::GetWindowLong( hWnd, GWL_USERDATA );

		switch( uMessage )
		{
			case WM_NCCREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
				pThis = (CDeskBand*)lpcs->lpCreateParams;
				::SetWindowLong( hWnd, GWL_USERDATA, (LONG)pThis );

				// set the window handle
				pThis->m_hWnd = hWnd;
			}
			break;

			case WM_CREATE:
				return pThis->OnCreate( (LPCREATESTRUCT)lParam );

			case WM_DESTROY:
				return pThis->OnDestroy();

			case WM_PAINT:
				return pThis->OnPaint();

			case WM_COMMAND:
				return pThis->OnCommand( wParam, lParam );

			case WM_SIZE:
				return pThis->OnSize( wParam, lParam );

			default:
				break;
		}

		return pThis->OnMsg( hWnd, uMessage, wParam, lParam );
	}
};

#endif // __ATLSHELL_H__
