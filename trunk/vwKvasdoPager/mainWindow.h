void setDefaults();
void canvasWindowUpdate();
HWND canvasWindowCreate();

// app window
HWND mainWindowHandle = 0;

extern HWND parent;

// initializes desktop sizes etc.
void mainWindowSetDefaults()
{
  RECT deskr,winr;
  int scrH, scrW;
    
  NUMDESKX = (int)SendMessage(vwHandle, VW_DESKX, 0, 0);  
  NUMDESKY = (int)SendMessage(vwHandle, VW_DESKY, 0, 0); 

  GetWindowRect(GetDesktopWindow(),&deskr);

  scrW = deskr.right;
  scrH = deskr.bottom;

  GetWindowRect(parent,&winr);

  if(winr.bottom-winr.top > 0)
    COEF = NUMDESKY*scrH/(winr.bottom-winr.top);  
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
    //if(parent)
    //  mainWindowSetDefaults();

    //if(canvasWindowHandle)
    //  canvasWindowUpdate();    
    // break; NO BREAK HERE, IT'S OK

  case WM_TIMER:
  case MOD_CFGCHANGE:
    if(!canvasWindowHandle)
    {
      if(canvasWindowCreate())
        tooltipCreate(canvasWindowHandle);
      return 0;
    }
    
    if(parent)
      if(vwHandle)
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