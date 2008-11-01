
char tiptext[80] = "";
HDC tipdc;
HWND tip = 0;
HFONT tipfont = 0;

extern HWND tipwin;

LRESULT CALLBACK
TooltipWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      tipdc = BeginPaint(hwnd, &ps);
      SelectObject(tipdc, tipfont);
      RECT r;
      GetWindowRect(hwnd, &r);
      Rectangle(tipdc, 0, 0, r.right-r.left, r.bottom-r.top);
      TextOut(tipdc,2, 2 ,tiptext,strlen(tiptext));
      EndPaint(hwnd, &ps);
    }
    return 0;

  case WM_TIMER:
    ShowWindow(tip, SW_HIDE);
    tipwin = 0;
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;  
}

void createTooltip(HWND hwndOwner) 
{ 
    /*HWND hwndTT;    // handle of tooltip 
    TOOLINFO ti;    // tool information 

    int id = 0;     // offset to string identifiers 
    static char *szTips[6] =   // tip text 
    { 
    "This is a tooltip"
    }; 
 
 
    hwndTT = CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU) NULL, 0, NULL); 

 
    if (hwndTT == (HWND) NULL) 
        return (HWND) NULL; 
 
    // Divide the client area into a grid of rectangles, and add each 
    // rectangle to the tooltip. 
    
    ti.cbSize = sizeof(TOOLINFO); 
    ti.uFlags = TTF_IDISHWND; 
    ti.hwnd = hwndOwner; 
    ti.hinst = 0; 
    ti.uId = (UINT) hwndOwner; 
    ti.lpszText = LPSTR_TEXTCALLBACK; 

    //ti.rect.left = 0; 
    //ti.rect.top = 0; 
    //ti.rect.right = 20; 
    //ti.rect.bottom = 20; 

    if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti)) 
        return NULL; 
         
 
    return hwndTT; */

  WNDCLASS wc;

  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)TooltipWndProc;
  wc.hInstance = 0;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
  this for locating the window */
  wc.lpszClassName = _T("NotATooltip");

  if (!RegisterClass(&wc))
  {
    MessageBox(0,_T("Cannot registed NotATooltip!"),_T("Module Plugin"), 0);
    //return;
  }

  RECT r;
  GetWindowRect(hwndOwner, &r);

  tip = CreateWindowEx(
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
} 

extern HWND mainw;
extern int WINW;
extern int overdesk;

bool enabledTip = 1;

void updateTip()
{
  if(!tip)
    return;

  if(!enabledTip)
    return;

//  strncpy(tiptext, text, 80);
  if(!tiptext[0])
  {
    ShowWindow(tip, SW_HIDE);
    return;
  }

  SIZE s;
  RECT r;
  HDC dc = GetWindowDC(tip);
  SelectObject(dc, tipfont);

  GetWindowRect(mainw, &r);

  GetTextExtentPoint(dc, tiptext, strlen(tiptext), &s);

  int x = r.left + (WINW*(overdesk-1) + WINW/2) - s.cx/2 - 3;
  if(x<5)
    x = 5;
  
  //SendMessage(w);
  //ShowWindow(tip, SW_SHOW);
  SetWindowPos(tip, HWND_TOPMOST, x, r.top-20, s.cx+6, s.cy+4, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
  InvalidateRect(tip, NULL, false);

  SetTimer(tip, 1, 1000, 0); // FIXME: ugly tooltip hack
}

disableTip()
{
  ShowWindow(tip, SW_HIDE);
  enabledTip = false;
}

enableTip()
{
  enabledTip = true;
}
