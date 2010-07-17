#include "stdafx.h"
#include "resource.h"
#include "sample.h"


// DeskBand example
//////////////////////////////////////////////////////////////
CSampleDeskBand::CSampleDeskBand()
{
}

CSampleDeskBand::~CSampleDeskBand()
{
}

LRESULT CSampleDeskBand::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc ;
    
    hdc = ::BeginPaint( m_hWnd, &ps );
    
    if(hdc && (m_iThemed > 0))
    {
        RECT rc;
        
        GetClientRect(m_hWnd, &rc);
        m_pDrawThemeParentBackground(m_hWnd,hdc,&rc) ;
    }
    
    ::EndPaint( m_hWnd, &ps );

    return 0;
}

LRESULT CSampleDeskBand::OnCommand( WPARAM wParam, LPARAM /*lParam*/ )
{
	//if( wParam == IDM_DESKBAND_CMD )
	//	::MessageBox( m_hWnd, TEXT("Sample Desk Band Command"), TEXT("Sample Desk Band"), MB_OK );

	return 0;
}

// Vertical Explorer Bar example
//////////////////////////////////////////////////////////////
/*CSampleVerticalBar::CSampleVerticalBar()
{
}

CSampleVerticalBar::~CSampleVerticalBar()
{
}

LRESULT CSampleVerticalBar::OnPaint()
{
	PAINTSTRUCT ps;

	::BeginPaint( m_hWnd, &ps );
	::SetTextColor( ps.hdc, RGB(255, 255, 255) );
	::SetBkMode( ps.hdc, TRANSPARENT );
	::DrawText( ps.hdc, TEXT("Sample Vertical Bar"), -1, &ps.rcPaint, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
	::EndPaint( m_hWnd, &ps );

	return 0;
}

// Horizontal Explorer Bar example
//////////////////////////////////////////////////////////////
CSampleHorizontalBar::CSampleHorizontalBar()
{
}

CSampleHorizontalBar::~CSampleHorizontalBar()
{
}

LRESULT CSampleHorizontalBar::OnPaint()
{
	PAINTSTRUCT ps;

	::BeginPaint( m_hWnd, &ps );
	::SetTextColor( ps.hdc, RGB(255, 255, 255) );
	::SetBkMode( ps.hdc, TRANSPARENT );
	::DrawText( ps.hdc, TEXT("Sample Horizontal Bar"), -1, &ps.rcPaint, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
	::EndPaint( m_hWnd, &ps );

	return 0;
}*/
