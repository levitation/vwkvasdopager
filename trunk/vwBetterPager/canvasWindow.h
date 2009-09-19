HWND createCanvasWindow();
void updateCanvasWindow();

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND canvasWindow = 0;		// main window

// Desktop drawing brushes
HBRUSH activeDesk, inactiveDesk, activeWindow, inactiveWindow; 
HPEN pagerFrame, activeWframe, inactiveWframe;


HFONT font;	// font for text rendering

HWND parent = 0;	// deskband window





HWND createCanvasWindow()
{
  WNDCLASS wc;

  parent = findWindowByClass(FindWindow("Shell_TrayWnd", NULL), "vwBetterPagerHost");
  if(parent == NULL)
    return 0;

  // set window class
  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.hInstance = 0;
  wc.hCursor = LoadCursor(0, IDC_ARROW);

  wc.lpszClassName = _T("vwBetterPagerCanvas");

  if (!RegisterClass(&wc))
    throw std::runtime_error("Cannot register canvas class!");

  canvasWindow = CreateWindowEx(
    0,                      // no extended styles
    _T("vwBetterPagerCanvas"),       // class name
    _T(""),           // window name
    WS_CHILD  ,
    0,          // default horizontal position
    0,          // default vertical position
    1,          // default width
    1,          // default height
    parent,            // no parent or owner window
    (HMENU) NULL,           // class menu used
    0,              // instance handle
    NULL);                  // no window creation data

  if (!canvasWindow)    
    throw std::runtime_error("Cannot create window!");

  // can't remember what this does, but this is undoubtfully important =)
  ShowWindow(canvasWindow, SW_HIDE);
  SetWindowLong(canvasWindow, GWL_EXSTYLE, GetWindowLong(canvasWindow, GWL_EXSTYLE) |
    WS_EX_TOOLWINDOW);
  ShowWindow(canvasWindow, SW_SHOW);

  UpdateWindow(canvasWindow);

  SetTimer(canvasWindow, 1, 300, NULL);

  return canvasWindow;
  

  //InitCommonControls();

  //tip = CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, 
  //      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
  //      canvasWindow, (HMENU) NULL, 0, NULL); 
  



  /*TRACKMOUSEEVENT tr;
  tr.cbSize = sizeof(tr);
  tr.hwndTrack = mainw;
  tr.dwFlags = TME_LEAVE;
  _TrackMouseEvent(&tr);*/
}

void createBrushes()
{
  font = CreateFont(
    8,
    0,
    GM_COMPATIBLE,
    0,
    FW_DONTCARE,
    FALSE,
    FALSE,
    FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    DRAFT_QUALITY,
    DEFAULT_PITCH,
    "CourierNew"
    );

  activeWindow = CreateSolidBrush(RGB(0x33,0x66,0x99));
  inactiveWindow = CreateSolidBrush(RGB(210,210,210));

  activeDesk = CreateSolidBrush(RGB(0x00,0x33,0x66));
  inactiveDesk = CreateSolidBrush(RGB(120,120,120));

  pagerFrame = CreatePen(PS_SOLID, 0, RGB(210,210,210));
  activeWframe = CreatePen(PS_SOLID, 0, RGB(255,255,255));
  inactiveWframe = CreatePen(PS_SOLID, 0, RGB(0,0,0));
}

void updateCanvasWindow()
{
  //TOOLINFO ti; 
  
  static HWND lastTip = 0;
 
  if(lastTip != tip)
  {
    SendMessage(vwHandle, VW_WINMANAGE, (WPARAM)tip, 0);
    lastTip = tip;
  }
  SetWindowPos(canvasWindow, HWND_TOPMOST, 0, 0, WINW*NUMDESKX+1, WINH*NUMDESKY, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

  /*ti.cbSize = sizeof(ti);
  ti.hinst = 0;
  ti.uId = 1;//(UINT)tip;
  ti.uFlags = 0;//TTF_IDISHWND;
  ti.lpszText = "hello";//LPSTR_TEXTCALLBACK;
  ti.hwnd = mainw;
  ti.rect.left = 0;
  ti.rect.top = 0;
  ti.rect.bottom = 100;
  ti.rect.right = 100;
  ti.lParam = 0;
  //if(tipinit == 0)
    PostMessage(tip, TTM_ADDTOOL, 0, (LPARAM)&ti); 
  //else
  //  PostMessage(tip, TTM_SETTOOLINFO, 0, (LPARAM)&ti); */
}

void deleteCanvasWindow()
{
  if(canvasWindow == 0)
    return;

  DeleteObject(activeWindow);
  DeleteObject(inactiveWindow);
  DeleteObject(activeDesk);
  DeleteObject(inactiveDesk);
  DeleteObject(pagerFrame);
  DeleteObject(font); 
}