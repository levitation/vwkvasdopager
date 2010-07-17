#include <Uxtheme.h>

void setDefaults();
void canvasWindowUpdate();
HWND canvasWindowCreate();

// app window
HWND mainWindowHandle = 0;

typedef BOOL (WINAPI *vwISTHEMEACTIVE)(void) ;
vwISTHEMEACTIVE vwIsThemeActive ;
typedef HRESULT (WINAPI *vwBUFFEREDPAINTINIT)(void) ;
vwBUFFEREDPAINTINIT vwBufferedPaintInit ;
typedef HRESULT (WINAPI *vwBUFFEREDPAINTUNINIT)(void) ;
vwBUFFEREDPAINTUNINIT vwBufferedPaintUnInit ;
typedef HPAINTBUFFER (WINAPI *vwBEGINBUFFEREDPAINT)(HDC,const RECT *,BP_BUFFERFORMAT,__in BP_PAINTPARAMS *,__out HDC *) ;
vwBEGINBUFFEREDPAINT vwBeginBufferedPaint ;
typedef HRESULT (WINAPI *vwENDBUFFEREDPAINT)(HPAINTBUFFER,BOOL) ;
vwENDBUFFEREDPAINT vwEndBufferedPaint ;
typedef DWORD (WINAPI *vwBUFFEREDPAINTSETALPHA)(HPAINTBUFFER, __in  const RECT *, BYTE) ;
vwBUFFEREDPAINTSETALPHA vwBufferedPaintSetAlpha ;
BOOL vwUseBufferedPaint=FALSE ;

extern HWND parent;

// initializes desktop sizes etc.
void mainWindowSetDefaults()
{
  RECT winr;
  int scrH, scrW, winH ;
    
  NUMDESKX = (int)SendMessage(vwHandle, VW_DESKX, 0, 0);  
  NUMDESKY = (int)SendMessage(vwHandle, VW_DESKY, 0, 0); 

  // Get screen 
  scrW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  scrH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

  GetWindowRect(parent,&winr);
    
  winH = winr.bottom-winr.top ;
    
  if((vwIsThemeActive != NULL) && vwIsThemeActive())
  {
    /* TODO: should check the location of the taskbar */
    winH -= 6 ;
    WINY = 4 ;
    if((vwBufferedPaintInit != NULL) && !vwUseBufferedPaint)
    {
      vwUseBufferedPaint = TRUE ;
      vwBufferedPaintInit() ;
    }
  }
  else
  {
    WINY = 0 ;
    if(vwUseBufferedPaint)
    {
      vwBufferedPaintUnInit() ;
      vwUseBufferedPaint = FALSE ;
    }
  }    
  if(winH > 0)
    COEF = NUMDESKY*scrH/winH ;
  else
    COEF = 1;

  WINW = scrW/COEF;
  WINH = scrH/COEF; 

  messages = 1;
}

LRESULT CALLBACK
mainWindowMessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case MOD_INIT:
    /* This must be taken care of in order to get the handle to VirtuaWin. */
    /* The handle to VirtuaWin comes in the wParam */
    vwHandle = (HWND) wParam; /* Should be some error handling here if NULL */            
    
    // break; NO BREAK HERE, IT'S OK

  case WM_TIMER:
  case MOD_CFGCHANGE:
    if(!canvasWindowHandle)
    {
      if(canvasWindowCreate())
        tooltipCreate(canvasWindowHandle);
      return 0;
    }
    /* no break */

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
  case WM_THEMECHANGED:

    if(parent && vwHandle)
        mainWindowSetDefaults();    

    if(canvasWindowHandle)
      canvasWindowUpdate();

    break;

  case MOD_QUIT:
    /* This must be handled, otherwise VirtuaWin can't shut down the module */
    PostQuitMessage(0);
    break;

  case MOD_SETUP:
    /* Optional */
    //MessageBox(hwnd,_T("Add setup here!"),_T("Module Plugin"), 0);
    break;

  case WM_DESTROY:        
    PostQuitMessage(0);
    break;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
    }
    return 0;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;  
}

/**
* Creates the main window
*/
HWND mainWindowCreate(HINSTANCE hInstance)
{
  WNDCLASS wc;

  // set window style
  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)mainWindowMessageHandler;
  wc.hInstance = hInstance ;

  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
  this for locating the window */
  wc.lpszClassName = _T("vwKvasdoPager.exe");

  // try to register window class
  if (!RegisterClass(&wc))
    throw std::runtime_error("Cannot register main window class");

  /* In this example, the window is never shown */
  mainWindowHandle = CreateWindow(_T("vwKvasdoPager.exe"),
    _T("vwkvasdoPager"), 
    WS_POPUP,
    CW_USEDEFAULT, 
    0, 
    CW_USEDEFAULT, 
    0,
    NULL,
    NULL,
    hInstance,
    NULL
  );
  
  if(!mainWindowHandle)
    throw std::runtime_error("Cannot create main window");
  
  SetTimer(mainWindowHandle, 1, 2000, NULL);
  return mainWindowHandle;
}