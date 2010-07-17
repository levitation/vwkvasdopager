HWND canvasWindowCreate();
void canvasWindowUpdate();

int redrawWindow(HWND hwnd);

HWND canvasWindowHandle = 0;		// main window

// Desktop drawing brushes
HBRUSH activeDesk, inactiveDesk, activeWindow, inactiveWindow; 
HPEN pagerFrame, activeWframe, inactiveWframe;


HFONT font;	// font for text rendering

HWND parent = 0;	// deskband window


#define WM_MOUSEWHEEL                   0x020A


// dragged window handler
HWND dragged = 0;
HWND draggedc = 0;
HWND mousedownHwnd = 0;
int mousedowndesk, overdesk, oldoverdesk;
int curdesk = 0;
int lastredraw = 0;


int oldx, oldy;

LRESULT CALLBACK
canvasWindowMessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if(messages == 0)
  {
    if(msg == WM_PAINT)
    {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
    }
    else
      return DefWindowProc(hwnd, msg, wParam, lParam);

    return 0;
  }

  switch (msg)
  {        
  case WM_TIMER:
    InvalidateRect(hwnd, NULL, FALSE);
    break;

  case WM_DESTROY:
    canvasWindowHandle = 0;
    parent = 0;
    break;

  case WM_PAINT:
    redrawWindow(hwnd);
    break;

  case WM_LBUTTONUP:
    {
      short xPos = LOWORD(lParam);  // horizontal position of cursor
      short yPos = HIWORD(lParam);  // vertical position of cursor   

      ReleaseCapture();

      // Bails out if cursor outside the pager
      RECT clientRect;
      GetClientRect(hwnd, &clientRect);
      if(xPos < 0 || yPos < 0 || xPos >= clientRect.right || yPos >= clientRect.bottom)
      {
        dragged = 0;
        draggedc = 0;
        return 0;
      }
      
      int dxPos = xPos/WINW;  // horizontal position of cursor
      int dyPos = yPos/WINH;  // vertical position of cursor    
      int desk = ((dxPos) + (dyPos)*NUMDESKX)+1;      

      if(dragged)
      {
        //MessageBeep(0);
        PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)dragged, desk);
        if(desk == curdesk)
          PostMessage(vwHandle, VW_FOREGDWIN, (WPARAM)dragged, 0);

        dragged = 0;
        draggedc = 0;
      }
      else
      {
        if(desk == curdesk)
        {
          //MessageBeep(0);

          int xmPos = LOWORD(lParam);  // horizontal position of cursor
          int ymPos = HIWORD(lParam);  // vertical position of cursor    

          while(xmPos > WINW)
            xmPos-=WINW;
          while(ymPos > WINH)
            ymPos-=WINH;

          HWND h = getWindowAt(xmPos*COEF, ymPos*COEF, desk, canvasWindowHandle);

          // Bring window to top if we've moused down on it
          if(h && h == mousedownHwnd)
          {
            //MessageBeep(0);
            BringWindowToTop(h);
          }
        }
        
        //MessageBeep(0);
        // MessageBox(hwnd,_T("Butn!"),_T("Module Plugin"), 0);
        //PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)hwnd, desk);
        //PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)tip, desk);
        //ShowWindow(tip, SW_HIDE);
        //tiptext[0] = 0;
        //updateTip();
        //disableTip();

        // Switch to desk if we've moused down on it
        if(mousedowndesk == desk)
          SendMessage(vwHandle, VW_CHANGEDESK, desk, desk);
        //enableTip();
        draggedc = 0;
        
      }
      InvalidateRect(hwnd, NULL, FALSE);
    }
    break;

  case WM_MOUSEMOVE:
    {     
      short xPos = tipXpos = LOWORD(lParam);  // horizontal position of cursor
      short yPos = tipYpos = HIWORD(lParam);  // vertical position of cursor   

      // Bails out if cursor outside the pager
      RECT clientRect;
      GetClientRect(hwnd, &clientRect);
      if(xPos < 0 || yPos < 0 || xPos >= clientRect.right || yPos >= clientRect.bottom)
      {
        overdesk = mousedowndesk;
        return 0;
      }
      
      int dxPos = xPos/WINW;  // horizontal position of cursor
      int dyPos = yPos/WINH;  // vertical position of cursor      

      if(dxPos >= NUMDESKX)
        dxPos = NUMDESKX-1;
      if(dyPos >= NUMDESKY)
        dyPos = NUMDESKY-1;

      overdesk = ((dxPos) + (dyPos)*NUMDESKX)+1;

      RECT mre;      
      GetWindowRect(canvasWindowHandle, &mre);
      //POINT pt = {xPos+mre.left, yPos+mre.top};

      //if(capture)
      //  if(!PtInRect(&mre, pt))
      //  {        
      //    ReleaseCapture(); // FIXME: косяк: после повторного захода в окно тултип не отображается
      //    tiptext[0] = 0;
      //    capture = 0;
      //    updateTip(xPos);
      //    break;
      //  }
      //else
      //{
      //  MessageBeep(0);
      //}

      {
        while(tipXpos > WINW)
          tipXpos-=WINW;
        while(tipYpos > WINH)
          tipYpos-=WINH;
        
        HWND htw;
        if(dragged)
          htw = dragged;
        else if(draggedc)
          htw = draggedc;
        else 
          htw = getWindowAt(tipXpos*COEF, tipYpos*COEF, overdesk, canvasWindowHandle);

        if(htw != tipwin)
        {          
          tipwin = htw;
          if(tipwin)
          {
            GetWindowText(tipwin, tiptext, sizeof(tiptext));
            //if(capture == 0)
           // {
            //  SetCapture(mainw);
            //  capture = 1;
            //}
          }
          else
            tiptext[0] = 0;
          tooltipUpdate();  
        }
      }     

      if(dragged)
      {
        
        if(oldoverdesk != overdesk)
        {                 
          oldoverdesk = overdesk;
          InvalidateRect(hwnd, NULL, FALSE);
        }
      }
      if(draggedc)
      {
        if(((oldx-xPos)*(oldx-xPos) + (oldy-yPos)*(oldy-yPos)) > 25)
        {
          
          dragged = draggedc;
          draggedc = 0;
          InvalidateRect(hwnd, NULL, FALSE);
        }
      }
    }

    break;

  case WM_LBUTTONDOWN:
    {
      int xPos = LOWORD(lParam);  // horizontal position of cursor
      int yPos = HIWORD(lParam);  // vertical position of cursor

      oldx = xPos;
      oldy = yPos;

      dragged = 0;
      //HWND h = GetForegroundWindow();
      //int curdesk = SendMessage(vwHandle, VW_CURDESK, 0, 0);
                         
      int dxpos = xPos/WINW;  // horizontal position of cursor
      int dypos = yPos/WINH;  // vertical position of cursor      

      if(dxpos >= NUMDESKX)
        dxpos = NUMDESKX-1;
      if(dypos >= NUMDESKY)
        dypos = NUMDESKY-1;

      int desk = ((dxpos) + (dypos)*NUMDESKX)+1;

      while(xPos > WINW)
        xPos-=WINW;
      while(yPos > WINH)
        yPos-=WINH;

      mousedowndesk = desk;

      HWND h = getWindowAt(xPos*COEF, yPos*COEF, desk, canvasWindowHandle);
      mousedownHwnd = h;
      if(!h)
        return 0;      

      //int flag  = (int)SendMessage(vwHandle, VW_WINGETINFO, (WPARAM) h, NULL);
      //int wdesk = vwWindowGetInfoDesk(flag);

      //char l[128];
      //sprintf(l, "[%d] %d+%d", desk, dxpos, dypos);
      //GetWindowText(h,l,128);
      //MessageBox(hwnd,l,_T("Module Plugin"), 0);

      //if(wdesk == desk)
      //{               
        // TODO: setcursor

        //if(curdesk == desk)
        //  BringWindowToTop(h);
       
        SetCapture(hwnd);

        dragged = 0;
        draggedc = h;
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
      //}
    } 
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;
}

HWND canvasWindowCreate()
{
  
  static bool isRegistered = 0;

  if(((parent = findWindowByClass(FindWindow("Shell_TrayWnd", NULL), "vwBetterPagerHost")) == NULL) &&
     ((parent = findWindowByClass(FindWindow("BaseBar", "KvasdoPager"), "vwBetterPagerHost")) == NULL))
    return 0;

  if(isRegistered == 0)
  {
    WNDCLASS wc;
    // set window class
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)canvasWindowMessageHandler;
    wc.hInstance = 0;
    wc.hCursor = LoadCursor(0, IDC_ARROW);

    wc.lpszClassName = _T("vwBetterPagerCanvas");

    if (!RegisterClass(&wc))
      throw std::runtime_error("Cannot register canvas class!");

    isRegistered = 1;
  }

  canvasWindowHandle = CreateWindowEx(
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

  if (!canvasWindowHandle)    
    throw std::runtime_error("Cannot create window!");

  // can't remember what this does, but this is undoubtfully important =)
  ShowWindow(canvasWindowHandle, SW_HIDE);
  SetWindowLong(canvasWindowHandle, GWL_EXSTYLE, GetWindowLong(canvasWindowHandle, GWL_EXSTYLE) |
    WS_EX_TOOLWINDOW);
  ShowWindow(canvasWindowHandle, SW_SHOW);

  UpdateWindow(canvasWindowHandle);

  SetTimer(canvasWindowHandle, 1, 300, NULL);

  return canvasWindowHandle;
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

void canvasWindowUpdate()
{
  //TOOLINFO ti; 
  
  static HWND lastTip = 0;
 
  if(lastTip != tooltipHandle)
  {
    SendMessage(vwHandle, VW_WINMANAGE, (WPARAM)tooltipHandle, 0);
    lastTip = tooltipHandle;
  }
  SetWindowPos(canvasWindowHandle, HWND_TOPMOST, 0, WINY, WINW*NUMDESKX+1, WINH*NUMDESKY, SWP_NOZORDER | SWP_NOACTIVATE);

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

void deleteBrushes()
{
  DeleteObject(activeWindow);
  DeleteObject(inactiveWindow);
  DeleteObject(activeDesk);
  DeleteObject(inactiveDesk);
  DeleteObject(pagerFrame);
  DeleteObject(font); 
}