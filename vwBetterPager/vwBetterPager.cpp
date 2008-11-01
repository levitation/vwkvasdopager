#include "stdafx.h"
#include "vwBetterPager.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "Messages.h"
#include "Defines.h"
//#include "commctrl.h"

#include "tipexample.h"


// FIXME: Иногда начинает безумено отжирать проц - FIXED
// TODO: При смене разрешения экрана, настроек программы, ширины статусбара надо обновлять настройки
// TODO: Сделать диалог с настройками
// TODO: Сделать всплывающую подсказку с заголовком наведённого окна
// FIXME: Иногда первое окно в системе не получается перетащить (например "Диспетчер задач")

void create2ndWindow();
void update2ndWindow();

int messages = 0;

int WINW = 1; // window picture width
int WINH = 1; // window picture heigt

int COEF = 1; // scale factor

int NUMDESKX; // desktop count
int NUMDESKY; // desktop count

// Desktop drawing brushes
HBRUSH activeDesk, inactiveDesk, activeWindow, inactiveWindow; 
HPEN pagerFrame, activeWframe, inactiveWframe;

// Window handlers
HWND dummy = 0;		// app window
HWND parent = 0;	// deskband window
HWND mainw = 0;		// main window
HWND vwHandle = 0;  // virtuawin

int tipXpos = 0;
int tipYpos = 0;

HFONT font;	// font for text rendering

HWND tipwin = 0;

// search for host window
HWND findWindow(HWND parent)
{
  HWND p,p2;
  char cls[1024];

  if(parent == NULL)
    return NULL;

  p = GetTopWindow(parent);
  while(p)
  {
    GetClassName(p, cls, sizeof(cls));
    if(strcmp(cls,"vwBetterPagerHost") == 0)
      return p;

    p2 = findWindow(p);
    if(p2)
      return p2;

    p = GetNextWindow(p, GW_HWNDNEXT);
  }

  return NULL;
}

// initializes desktop sizes etc.
void setDefaults()
{
  RECT deskr,winr;
  int scrH, scrW;
    
  NUMDESKX = (int)SendMessage(vwHandle, VW_DESKX, 0, 0);  
  NUMDESKY = (int)SendMessage(vwHandle, VW_DESKY, 0, 0); 

  GetWindowRect(GetDesktopWindow(),&deskr);

  scrW = deskr.right;
  scrH = deskr.bottom;

  GetWindowRect(parent,&winr);

  COEF = NUMDESKY*scrH/(winr.bottom-winr.top);  

  WINW = scrW/COEF;
  WINH = scrH/COEF;   
}

void fixRect(RECT* r)
{
  if(r->left < -20000)
  {
    r->left+=25000;
    r->right+=25000;
  }
  if(r->top < -20000)
  {
    r->top+=25000;
    r->bottom+=25000;
  }
}

void clipRect(RECT* r)
{
  if(r->left < 0)
    r->left=0;
  if(r->top < 0)
    r->top=0;
  if(r->right > WINW*COEF)
    r->right = WINW*COEF;
  if(r->bottom > WINH*COEF)
    r->bottom = WINH*COEF;
}

// stuff for rendering
HDC hdc,ohdc;
RECT dcr;
HBITMAP hBmp;
PAINTSTRUCT ps;

// dragged window handler
HWND dragged = 0;
HWND draggedc = 0;
int dragdesk, overdesk, oldoverdesk;
int curdesk = 0;
// main rendering procedure
int redrawWindow(HWND hwnd)
{
  RECT wrc;
  HWND h, hOld;
  int i,j;
  
  ohdc = BeginPaint(mainw, &ps);
  hdc = CreateCompatibleDC(ohdc);
  GetClientRect(mainw, &dcr);
  hBmp = CreateCompatibleBitmap(ohdc, dcr.right-dcr.left, dcr.bottom-dcr.top);
  SelectObject(hdc,hBmp);
  SelectObject(hdc, font);
  SetBkMode(hdc, TRANSPARENT);

  curdesk = (int)SendMessage(vwHandle, VW_CURDESK, 0, 0);

  // draw desktop backgrounds
  for(i=0;i<NUMDESKY;i++)
  {
    for(j=0;j<NUMDESKX;j++)
    {
      if((i*NUMDESKX+j)+1 == curdesk)
        SelectObject(hdc, activeDesk);
      else
        SelectObject(hdc, inactiveDesk);

      Rectangle(hdc,WINW*j,WINH*i,WINW*(j+1)+1,WINH*(i+1)+1);
    }
  }

  // find last window
  hOld = h = GetForegroundWindow();
  while(h)
  {
    hOld = h;
    h = GetNextWindow(h, GW_HWNDNEXT);
  }
  h = hOld;

  //draw windows
  while(h)
  {
    RECT r;  
    if(h != hwnd) // not render myself
    {
      if(GetWindowRect(h, &r))
      {
        char text[vwWINDOWNAME_MAX];

        // get window desktop
        int flag  = (int)SendMessage(vwHandle, VW_WINGETINFO, (WPARAM) h, NULL);

        // if window is managed
        if(flag)
        {
          // dragged window is rendered last          
          if(h != dragged)
          {
            // TODO: add hung vindow display
            // TODO: add non-managed window display

            int desk = vwWindowGetInfoDesk(flag);
            //desk = desk-1;

            // handle vw's "-25000 shift" trick
            fixRect(&r);
            
            // get window caption
            if(!GetWindowText(h,text,vwWINDOWNAME_MAX))
              text[0] = 0;

            // select brush

			if(h == GetForegroundWindow())
            //if(desk == curdesk)
			{
              SelectObject(hdc, activeWindow);
			  SelectObject(hdc, activeWframe);
			}
            else
			{
              SelectObject(hdc, inactiveWindow);          
			  SelectObject(hdc, inactiveWframe);
			}
          
            RECT tr;
            int deskx = (desk-1)%NUMDESKX;
            int desky = (desk-1)/NUMDESKX; 

			r.left /= COEF;
			r.right /= COEF;
			r.top /= COEF;
			r.bottom /= COEF;

			if(r.left<1)
				r.left = 1;
			if(r.top<1)
				r.top = 1;
			if(r.right > WINW-1)
				r.right = WINW-1;
			if(r.bottom > WINH-1)
				r.bottom = WINH-1;

            tr.left = r.left+WINW*deskx+1;
            tr.top = r.top+WINH*desky+1;
            tr.right = r.right+WINW*deskx;
            tr.bottom = r.bottom+WINH*desky;

            Rectangle(hdc, r.left+WINW*deskx, r.top+WINH*desky, r.right+WINW*deskx+1, r.bottom+WINH*desky);
			if((r.right-r.left > 16) && (r.bottom - r.top) > 16)
			{

				DrawIconEx(
					hdc, 
					(r.left + (r.right - r.left)/2)+WINW*deskx - 7, 
					(r.top + (r.bottom - r.top)/2)+WINH*desky - 7, 
					(HICON)GetClassLong(h,GCL_HICON), 
					16, 16, 
					0, 0, 
					DI_NORMAL
				);
			}
            //DrawText(hdc, text, (int)strlen(text), &tr, DT_LEFT	| DT_TOP | DT_SINGLELINE);          
          }
        }
      }
    }
    h = GetNextWindow(h, GW_HWNDPREV);
  }

  if(dragged)
  {
    RECT r;
    RECT tr;
    char text[vwWINDOWNAME_MAX] ;

    GetWindowRect(dragged, &r);
    
    fixRect(&r);

    if(!GetWindowText(dragged,text,vwWINDOWNAME_MAX))
      text[0] = 0;
    
    SelectObject(hdc, activeWindow);

    int deskx = (overdesk-1)%NUMDESKX;
    int desky = (overdesk-1)/NUMDESKX; 

	r.left /= COEF;
	r.right /= COEF;
	r.top /= COEF;
	r.bottom /= COEF;

	if(r.left<1)
		r.left = 1;
	if(r.top<1)
		r.top = 1;
	if(r.right > WINW-1)
		r.right = WINW-1;
	if(r.bottom > WINH-1)
		r.bottom = WINH-1;

    tr.left = r.left+WINW*deskx+1;
    tr.top = r.top+WINH*desky+1;
    tr.right = r.right+WINW*deskx;
    tr.bottom = r.bottom+WINH*desky;

    Rectangle(hdc, r.left+WINW*deskx, r.top+WINH*desky, r.right+WINW*deskx+1, r.bottom+WINH*desky);
    if((r.right-r.left > 16) && (r.bottom - r.top) > 16)
			{

				DrawIconEx(
					hdc, 
					(r.left + (r.right - r.left)/2)+WINW*deskx - 7, 
					(r.top + (r.bottom - r.top)/2)+WINH*desky - 7, 
					(HICON)GetClassLong(dragged,GCL_HICON), 
					16, 16, 
					0, 0, 
					DI_NORMAL
				);
			}
	//DrawText(hdc, text, (int)strlen(text), &tr, DT_LEFT	| DT_TOP | DT_SINGLELINE);
  }

  GetClientRect(hwnd, &wrc);

  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  SelectObject(hdc, pagerFrame);
    // draw desktop backgrounds
  for(i=0;i<NUMDESKY;i++)
  {
    for(j=0;j<NUMDESKX;j++)
    {
      Rectangle(hdc,WINW*j,WINH*i,WINW*(j+1)+1,WINH*(i+1)+1);
    }
  }  
  Rectangle(hdc, wrc.left, wrc.top, wrc.right, wrc.bottom);

  BitBlt(ohdc, 0, 0, dcr.right - dcr.left, dcr.bottom-dcr.top, hdc, 0, 0, SRCCOPY);

  DeleteDC(hdc);
  DeleteObject(hBmp);
  EndPaint(mainw, &ps);

  return 0L;
}

int oldx, oldy;

LRESULT CALLBACK
DummyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case MOD_INIT:
    /* This must be taken care of in order to get the handle to VirtuaWin. */
    /* The handle to VirtuaWin comes in the wParam */
    vwHandle = (HWND) wParam; /* Should be some error handling here if NULL */        
    setDefaults();    
    update2ndWindow();
    messages = 1;
    // break; NO BREAK HERE, IT'S OK

  case WM_TIMER:
  case MOD_CFGCHANGE:
    if(!mainw)
      return 0;
    setDefaults();    
    update2ndWindow();
    break;

  case MOD_QUIT:
    /* This must be handled, otherwise VirtuaWin can't shut down the module */
    PostQuitMessage(0);
    break;

  case MOD_SETUP:
    /* Optional */
    MessageBox(hwnd,_T("Add setup here!"),_T("Module Plugin"), 0);
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

HWND getWindowAt(int x, int y, int desk)
{
  HWND h = GetForegroundWindow(); 
  POINT pt = {x, y};
  while(h)
  {
    int flag  = (int)SendMessage(vwHandle, VW_WINGETINFO, (WPARAM) h, NULL);
    if(vwWindowGetInfoDesk(flag) == desk)
    {
      if(h != mainw)
      {
        RECT r;        
        GetWindowRect(h, &r);

        if(r.left < -20000)
        {
          r.left+=25000;
          r.right+=25000;
        }
        if(r.top < -20000)
        {
          r.top+=25000;
          r.bottom+=25000;
        }

        if(PtInRect(&r, pt))
        {                   
          return h;
        }
      }
    }
    h = GetNextWindow(h, GW_HWNDNEXT);
  }
  return 0;
}

int capture = 0;

#define WM_MOUSEWHEEL                   0x020A

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

  /*if(tip)
  {
    switch (msg) 
    { 
    case WM_MOUSEMOVE: 
    case WM_LBUTTONDOWN: 
    case WM_LBUTTONUP: 
    case WM_RBUTTONDOWN: 
    case WM_RBUTTONUP: 
      {
        MSG message; 

        message.lParam = lParam; 
        message.wParam = wParam; 
        message.message = msg; 
        message.hwnd = mainw; 
        SendMessage(tip, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &message); 
      } 
      break;

    default:
      break;
    }
  }*/


  switch (msg)
  {        
  case WM_TIMER:
    /*if(GetCapture() != mainw)
    {      
      //MessageBeep(0);
      
    }*/
    InvalidateRect(hwnd, NULL, FALSE);
    //MessageBeep(0);
    //redrawWindow(hwnd);
    break;

  case WM_PAINT:
    //
    //redrawWindow(hwnd);
    {
    //PAINTSTRUCT ps;
    //BeginPaint(hwnd, &ps);
    //EndPaint(hwnd, &ps);
    redrawWindow(hwnd);
    //MessageBeep(0);
    }
    break;

  /*case WM_NOTIFY:
    if ((((LPNMHDR) lParam)->code) == TTN_NEEDTEXT) 
    { 
      LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam; 

      while(tipXpos > WINW)
        tipXpos-=WINW;
      while(tipYpos > WINH)
        tipYpos-=WINH;

      HWND h = getWindowAt(tipXpos*COEF, tipYpos*COEF, overdesk);
      if(!h)
        lpttt->szText[0] = 0;
      else
        GetWindowText(h, lpttt->szText, sizeof(lpttt->szText));
      MessageBeep(0);
      {
        SendMessage(tip, TTM_UPDATETIPTEXT, );
      }
    }
    break;*/

  case WM_LBUTTONUP:
    {
      int xPos = LOWORD(lParam)/WINW;  // horizontal position of cursor
      int yPos = HIWORD(lParam)/WINH;  // vertical position of cursor      

      if(xPos >= NUMDESKX)
        xPos = NUMDESKX-1;
      if(yPos >= NUMDESKY)
        yPos = NUMDESKY-1;

      int desk = ((xPos) + (yPos)*NUMDESKX)+1;      

      if(dragged)
      {
        //MessageBeep(0);
        PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)dragged, desk);
        BringWindowToTop(dragged);
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

          HWND h = getWindowAt(xmPos*COEF, ymPos*COEF, desk);
          if(h)
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
        SendMessage(vwHandle, VW_CHANGEDESK, desk, desk);
        //enableTip();
        draggedc = 0;
        
      }
      InvalidateRect(hwnd, NULL, FALSE);
    }
    break;

  /*case WM_MOUSEWHEEL:
    {
      int zDelta = (short) HIWORD(wParam);
      if(zDelta > 0)
      {
        SendMessage(vwHandle, VW_STEPNEXT, 0, 0);
      }
      if(zDelta < 0)
      {
        SendMessage(vwHandle, VW_STEPPREV, 0, 0);
      }
    }
    break;*/

  case WM_MOUSEMOVE:
    {     
      int xPos = tipXpos = LOWORD(lParam);  // horizontal position of cursor
      int yPos = tipYpos = HIWORD(lParam);  // vertical position of cursor            
      
      int dxPos = LOWORD(lParam)/WINW;  // horizontal position of cursor
      int dyPos = HIWORD(lParam)/WINH;  // vertical position of cursor      

      if(dxPos >= NUMDESKX)
        dxPos = NUMDESKX-1;
      if(dyPos >= NUMDESKY)
        dyPos = NUMDESKY-1;

      overdesk = ((dxPos) + (dyPos)*NUMDESKX)+1;

      RECT mre;      
      GetWindowRect(mainw, &mre);
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
          htw = getWindowAt(tipXpos*COEF, tipYpos*COEF, overdesk);

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
          updateTip();  
        }
      }     

      if(dragged)
      {
        //MessageBeep(0);
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
          //MessageBeep(0);
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

      HWND h = getWindowAt(xPos*COEF, yPos*COEF, desk);
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
       
        dragdesk = desk;
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

void create2ndWindow()
{
  WNDCLASS wc;

  parent = findWindow(FindWindow("Shell_TrayWnd", NULL));
  if(parent == NULL)
  {
    MessageBox(0,_T("Tray not found!"),_T("Module Plugin"), 0);
    return;
  }  

  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.hInstance = 0;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
  this for locating the window */
  wc.lpszClassName = _T("vwBetterPager");

  if (!RegisterClass(&wc))
  {
    MessageBox(mainw,_T("Cannot registed vwBetterPager!"),_T("Module Plugin"), 0);
    //return;
  }

  mainw = CreateWindowEx(
    0,                      // no extended styles
    _T("vwBetterPager"),       // class name
    _T(""),           // window name
    WS_CHILD  ,//WS_ |   // overlapped window
    //WS_HSCROLL |        // horizontal scroll bar
    //WS_VSCROLL,         // vertical scroll bar
    0,          // default horizontal position
    0,          // default vertical position
    1,          // default width
    1,          // default height
    parent,            // no parent or owner window
    (HMENU) NULL,           // class menu used
    0,              // instance handle
    NULL);                  // no window creation data

  if (!mainw)    
  {
    MessageBox(mainw,_T("Cannot create window!"),_T("Module Plugin"), 0);
    //return;
  }

  ShowWindow(mainw, SW_HIDE);
  SetWindowLong(mainw, GWL_EXSTYLE, GetWindowLong(mainw, GWL_EXSTYLE) |
    WS_EX_TOOLWINDOW);
  ShowWindow(mainw, SW_SHOW);

  //ShowWindow(hwnd, SW_SHOWDEFAULT);
  UpdateWindow(mainw);

  SetTimer(mainw, 1, 300, NULL);
  //SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE	| SWP_NOSIZE);

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

  //InitCommonControls();

  //tip = CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, 
  //      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
  //      mainw, (HMENU) NULL, 0, NULL); 
  createTooltip(mainw);



  /*TRACKMOUSEEVENT tr;
  tr.cbSize = sizeof(tr);
  tr.hwndTrack = mainw;
  tr.dwFlags = TME_LEAVE;
  _TrackMouseEvent(&tr);*/
}

//int tipinit = 0;

void update2ndWindow()
{
  //TOOLINFO ti; 
  
  SendMessage(vwHandle, VW_WINMANAGE, (WPARAM)tip, 0); 
  SetWindowPos(mainw, HWND_TOPMOST, 0, 0, WINW*NUMDESKX+1, WINH*NUMDESKY, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

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




void delete2ndWindow()
{
  if(mainw == 0)
    return;

  DeleteObject(activeWindow);
  DeleteObject(inactiveWindow);
  DeleteObject(activeDesk);
  DeleteObject(inactiveDesk);
  DeleteObject(pagerFrame);
  DeleteObject(font); 
}

/*
* Main startup function
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
  WNDCLASS wc;
  MSG msg;

  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)DummyWndProc;
  wc.hInstance = hInstance ;
  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
  this for locating the window */
  wc.lpszClassName = _T("vwBetterPager.exe");

  if (!RegisterClass(&wc))
  {
    MessageBox(0,_T("Cannot registed vwBetterPager.exe!"),_T("Module Plugin"), 0);
    return 0;
  }

  /* In this example, the window is never shown */
  dummy = CreateWindow(_T("vwBetterPager.exe"),
    _T("vwBetterPager"), 
    WS_POPUP,
    CW_USEDEFAULT, 
    0, 
    CW_USEDEFAULT, 
    0,
    NULL,
    NULL,
    hInstance,
    NULL);

  if(!dummy)
  {
    MessageBox(0,_T("Cannot create dummy!"),_T("Module Plugin"), 0);
    return FALSE;
  }  

  SetTimer(dummy, 1, 2000, NULL);

  create2ndWindow();

  /* main messge loop */
  while (GetMessage(&msg, NULL, 0, 0) != 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  delete2ndWindow();

  return (int)msg.wParam;
}
