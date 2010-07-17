
char tiptext[80] = "";
HWND tooltipHandle = 0;
HFONT tipfont = 0;

extern HWND tipwin;

void tooltipPaint()
{
  PAINTSTRUCT ps;
  RECT r;
  HDC dc;

  dc = BeginPaint(tooltipHandle, &ps);
  SelectObject(dc, tipfont);
  
  GetWindowRect(tooltipHandle, &r);
  Rectangle(dc, 0, 0, r.right-r.left, r.bottom-r.top);
  TextOut(dc,3, 2, tiptext, (int) strlen(tiptext));
  EndPaint(tooltipHandle, &ps);
}

LRESULT CALLBACK
tooltipProcessMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_PAINT:
    tooltipPaint();
    return 0;

  case WM_TIMER:
    ShowWindow(tooltipHandle, SW_HIDE);
    tipwin = 0;
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;  
}

HWND tooltipCreate(HWND hwndOwner) 
{
  static bool isRegistered = 0;

  if(isRegistered == 0)
  {
    WNDCLASS wc;

    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)tooltipProcessMessages;
    wc.hInstance = 0;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
    this for locating the window */
    wc.lpszClassName = _T("NotATooltip");

    if (!RegisterClass(&wc))
      throw std::runtime_error("Cannot register window NotATooltip!");

    isRegistered = 1;
  }

  tooltipHandle = CreateWindowEx(
    0,                      // no extended styles
    _T("NotATooltip"),       // class name
    _T(""),           // window name
    WS_POPUP | WS_EX_TOPMOST,//WS_ |   // overlapped window
    //WS_HSCROLL |        // horizontal scroll bar
    //WS_VSCROLL,         // vertical scroll bar
    0,          // default horizontal position
    0,          // default vertical position
    1,          // default width
    1,          // default height
    hwndOwner,            // no parent or owner window
    (HMENU) NULL,           // class menu used
    0,              // instance handle
    NULL);                  // no window creation data

  tipfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

  return tooltipHandle;
} 

extern HWND canvasWindowHandle;
extern int WINW;
extern int overdesk;

bool tooltipEnabled = 1;

void tooltipUpdate()
{
  if(!tooltipHandle)
    return;

  if(!tooltipEnabled)
    return;

//  strncpy(tiptext, text, 80);
  if(!tiptext[0])
  {
    ShowWindow(tooltipHandle, SW_HIDE);
    return;
  }

  SIZE s;
  RECT r;
  HDC dc = GetWindowDC(tooltipHandle);
  SelectObject(dc, tipfont);

  GetWindowRect(canvasWindowHandle, &r);

  GetTextExtentPoint(dc, tiptext, (int) strlen(tiptext), &s);

  ReleaseDC(tooltipHandle, dc);

  int x = r.left + (WINW*(overdesk-1) + WINW/2) - s.cx/2 - 3;
  if(x<5)
    x = 5;
  
  //SendMessage(w);
  //ShowWindow(tip, SW_SHOW);
  SetWindowPos(tooltipHandle, HWND_TOPMOST, x, r.top-20, s.cx+6, s.cy+4, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
  InvalidateRect(tooltipHandle, NULL, false);

  SetTimer(tooltipHandle, 1, 1000, 0); // FIXME: ugly tooltip hack
}

void tooltipDisable()
{
  ShowWindow(tooltipHandle, SW_HIDE);
  tooltipEnabled = false;
}

void tooltipEnable()
{
  tooltipEnabled = true;
}
