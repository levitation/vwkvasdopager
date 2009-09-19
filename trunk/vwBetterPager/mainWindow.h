#include <iostream>

// app window
HWND mainWindow = 0;

LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
* Creates the main window
*/
int createMainWindow(HINSTANCE hInstance)
{
  WNDCLASS wc;

  // set window style
  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)DummyWndProc;
  wc.hInstance = hInstance ;

  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
  this for locating the window */
  wc.lpszClassName = _T("vwBetterPager.exe");

  // try to register window class
  if (!RegisterClass(&wc))
    throw std::runtime_error("Cannot register main window class");

  /* In this example, the window is never shown */
  mainWindow = CreateWindow(_T("vwBetterPager.exe"),
    _T("vwBetterPager"), 
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
  
  if(!mainWindow)
    throw std::runtime_error("Cannot create main window");
  
  SetTimer(mainWindow, 1, 2000, NULL);
}