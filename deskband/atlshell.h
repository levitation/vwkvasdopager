#ifndef __ATLSHELL_H__
#define __ATLSHELL_H__

#ifndef __cplusplus
#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atlbase.h>
#include <shlobj.h>
#include <comdef.h>
#include <Strsafe.h>

typedef BOOL (WINAPI *vwISTHEMEACTIVE)(void) ;
typedef HRESULT (WINAPI *vwDRAWTHEMEPARENTBACKGROUND)(__in  HWND, __in  HDC, __in  const RECT *);

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
	public IObjectWithSite, public IPersistStream, public IDeskBand2/*, IContextMenu*/
{
public:
	CDeskBand() : m_pSite( NULL ), m_hWnd( NULL ), m_iThemed( 0 ), m_pIsThemeActive( NULL ), m_pDrawThemeParentBackground( NULL ) {}

	BEGIN_COM_MAP(CDeskBand)
		COM_INTERFACE_ENTRY(IObjectWithSite)
		COM_INTERFACE_ENTRY(IPersist)
		COM_INTERFACE_ENTRY(IPersistStream)
		COM_INTERFACE_ENTRY(IOleWindow)
		COM_INTERFACE_ENTRY(IDockingWindow)
		COM_INTERFACE_ENTRY(IDeskBand)
		COM_INTERFACE_ENTRY(IDeskBand2)
//		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	// IObjectWithSite methods
	STDMETHOD(SetSite)( IUnknown *punkSite )
	{
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
					wc.style		= CS_HREDRAW | CS_VREDRAW;
					wc.lpfnWndProc		= (WNDPROC)CDeskBand::WndProc;
					wc.cbClsExtra		= 0;
					wc.cbWndExtra		= 0;
					wc.hInstance		= _Module.m_hInst;
					wc.hIcon		= NULL;
					wc.hCursor		= ::LoadCursor( NULL, IDC_ARROW );
					wc.hbrBackground	= (HBRUSH) (COLOR_BTNFACE+1);
					wc.lpszMenuName		= NULL;
					wc.lpszClassName	= lpClassName;

					::RegisterClass( &wc ); // if RegisterClass fails, CreateWindow below will fail
				}

				RECT rc;
				::GetClientRect( hWndParent, &rc );

				// create the window, the WndProc will set m_hWnd
				::CreateWindowEx(WS_EX_TRANSPARENT, lpClassName, NULL, WS_CHILD,
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWndParent, NULL, _Module.m_hInst, (LPVOID)this );
			}

			if( ! m_hWnd )
				return E_FAIL;
			
			if(m_iThemed >= 0)
			{
				HINSTANCE libHandle ; 
				if((m_pIsThemeActive == NULL) &&
				   (((libHandle = LoadLibrary(_T("UxTheme"))) == NULL) ||
				    ((m_pDrawThemeParentBackground = (vwDRAWTHEMEPARENTBACKGROUND) GetProcAddress(libHandle,"DrawThemeParentBackground")) == NULL) ||
				    ((m_pIsThemeActive = (vwISTHEMEACTIVE) GetProcAddress(libHandle,"IsThemeActive")) == NULL)))
					m_iThemed = -1 ;
				else
					m_iThemed = (m_pIsThemeActive()) ? 1:0 ;
			}

			// Hold on this object
			punkSite->AddRef();
		}

		// Release old site pointer
		if( m_pSite )
			m_pSite->Release();

		// keep new site pointer or NULL
		m_pSite = punkSite;


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
                    pdbi->ptMinSize.x = 10;
                    pdbi->ptMinSize.y = 10;
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
			pdbi->ptActual.x = 0;
			pdbi->ptActual.y = 0;
		}

		if( pdbi->dwMask & DBIM_TITLE )
                {
			if( dwViewMode & DBIF_VIEWMODE_FLOATING )
                            StringCbCopyW(pdbi->wszTitle, sizeof(pdbi->wszTitle), L"KvasdoPager");
                        else
                            pdbi->dwMask &= ~DBIM_TITLE;
                }
            
		if( pdbi->dwMask & DBIM_MODEFLAGS )
		{
			pdbi->dwModeFlags = DBIMF_NORMAL;
			pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
		}

		if( pdbi->dwMask & DBIM_BKCOLOR )
			pdbi->dwMask &= ~DBIM_BKCOLOR;

		return S_OK;
	}

	// IDeskband2 Methods
	STDMETHOD(CanRenderComposited)( BOOL *pfCanRenderComposited )
	{
		*pfCanRenderComposited = TRUE;
		return S_OK;
	}

	STDMETHOD(SetCompositionState)( BOOL fCompositionEnabled )
	{
		this->compositionEnabled = fCompositionEnabled;
		return S_OK;
	}

	STDMETHOD(GetCompositionState)( BOOL *pfCompositionEnabled )
	{
		*pfCompositionEnabled = this->compositionEnabled;
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
	BOOL compositionEnabled;
	int m_iThemed;
	vwISTHEMEACTIVE m_pIsThemeActive;
	vwDRAWTHEMEPARENTBACKGROUND m_pDrawThemeParentBackground;

private:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
	{
#ifdef WIN64
		CDeskBand *pThis = (CDeskBand*)::GetWindowLongPtr( hWnd, GWLP_USERDATA );
#else
		CDeskBand *pThis = (CDeskBand*)::GetWindowLong( hWnd, GWL_USERDATA );
#endif
		switch( uMessage )
		{
			case WM_NCCREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
				pThis = (CDeskBand*)lpcs->lpCreateParams;
#ifdef WIN64
				::SetWindowLongPtr( hWnd, GWLP_USERDATA, (size_t)pThis );
#else
				::SetWindowLong( hWnd, GWL_USERDATA, (LONG)pThis );
#endif
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

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
			case WM_THEMECHANGED:
				if(pThis->m_pIsThemeActive != NULL)
					pThis->m_iThemed = (pThis->m_pIsThemeActive()) ? 1:0 ;
				break;

			default:
				break;
		}

		return pThis->OnMsg( hWnd, uMessage, wParam, lParam );
	}
};

#endif // __ATLSHELL_H__
