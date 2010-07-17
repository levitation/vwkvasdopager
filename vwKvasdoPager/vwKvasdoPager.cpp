#include "stdafx.h"
#include "vwKvasdoPager.h"

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include "Messages.h"
#include "Defines.h"
#include <time.h>
#include <iostream>
//#include "commctrl.h"

int messages = 0;

int WINW = 1; // window picture width
int WINH = 1; // window picture heigt

int WINY = 0; // window picture Y offest

int COEF = 1; // scale factor

int NUMDESKX; // desktop count
int NUMDESKY; // desktop count

// Window handlers
HWND vwHandle = 0;  // virtuawin

int tipXpos = 0;
int tipYpos = 0;


HWND tipwin = 0;


#include "utils.h"
#include "tooltip.h"
#include "mainWindow.h"
#include "canvasWindow.h"
#include "mouseHook.h"


// stuff for rendering
RECT dcr;
HBITMAP hBmp;
PAINTSTRUCT ps;


// main rendering procedure
int redrawWindow(HWND hwnd)
{
  HPAINTBUFFER hBufferedPaint ;
  RECT wrc;
  HWND h, hOld;
  HDC hdc,ohdc;
  int i,j;
 
  int now = GetTickCount();
  if(now - lastredraw < 100)
    return 0;

  lastredraw = now;
  ohdc = BeginPaint(canvasWindowHandle, &ps);
  GetClientRect(canvasWindowHandle, &dcr);
  if(vwUseBufferedPaint)
  {
    BP_PAINTPARAMS paintParams = {0};
    paintParams.cbSize = sizeof(paintParams);
    hBufferedPaint = vwBeginBufferedPaint(ohdc, &dcr, BPBF_TOPDOWNDIB, &paintParams, &hdc);
  }
  else
  {
    hdc = CreateCompatibleDC(ohdc);
    hBmp = CreateCompatibleBitmap(ohdc, dcr.right-dcr.left, dcr.bottom-dcr.top);
    SelectObject(hdc,hBmp);
  }

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
  if((h = GetForegroundWindow()) == NULL)
    h = GetTopWindow(NULL);
  hOld = h;
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
      if(multiMonGetWindowRect(h, &r))
      {
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
            
            int ww, hh, ll, tt ;
            if(((ww=r.right-r.left) > 14) && ((hh=r.bottom - r.top) > 14))
            {
              HICON icn ;
              if((icn = (HICON) GetClassLong(h, GCL_HICON)) == NULL)
              {
                // Fallback plan, maybe this works better for this type of application
                // Otherwise there is not much we can do (could try looking for an owned window)
                // Note: some apps (e.g. Opera) only have big icons
                DWORD dd ;
                if((SendMessageTimeout(h, WM_GETICON, ICON_SMALL, 0L, 
                                       SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &dd) && (dd != 0)) ||
                   (SendMessageTimeout(h, WM_GETICON, ICON_BIG, 0L, 
                                       SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &dd) && (dd != 0)))
                  icn = (HICON) dd ;
              }
              if(icn != NULL)
              {
                if(ww > 16)
                {
                  ll = r.left + ww/2 - 7 ;
                  ww = 16 ;
                }
                else
                {
                  ll = r.left + 1 ;
                  ww-- ;
                }   
                if(hh > 16)
                {
                  tt = r.top + hh/2 - 7 ;
                  hh = 16 ;
                }
                else
                {
                  tt = r.top + 1 ;
                  hh-- ;
                }   
                DrawIconEx(hdc, 
                           ll+WINW*deskx, 
                           tt+WINH*desky, 
                           icn,
                           ww, hh,
                           0, 0, 
                           DI_NORMAL
                           );
              }
            }
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

    multiMonGetWindowRect(dragged, &r);
    
    fixRect(&r);

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
    
    int ww, hh, ll, tt ;
    if(((ww=r.right-r.left) > 14) && ((hh=r.bottom - r.top) > 14))
    {
      HICON icn ;
      if((icn = (HICON) GetClassLong(dragged, GCL_HICON)) == NULL)
      {
        // Fallback plan, maybe this works better for this type of application
        // Otherwise there is not much we can do (could try looking for an owned window)
        // Note: some apps (e.g. Opera) only have big icons
        DWORD dd ;
        if((SendMessageTimeout(dragged, WM_GETICON, ICON_SMALL, 0L, 
                               SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &dd) && (dd != 0)) ||
           (SendMessageTimeout(dragged, WM_GETICON, ICON_BIG, 0L, 
                               SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &dd) && (dd != 0)))
          icn = (HICON) dd ;
      }
      if(icn != NULL)
      {
        if(ww > 16)
        {
          ll = r.left + ww/2 - 7 ;
          ww = 16 ;
        }
        else
        {
          ll = r.left + 1 ;
          ww-- ;
        }   
        if(hh > 16)
        {
          tt = r.top + hh/2 - 7 ;
          hh = 16 ;
        }
        else
        {
          tt = r.top + 1 ;
          hh-- ;
        }   
        DrawIconEx(hdc, 
                   ll+WINW*deskx, 
                   tt+WINH*desky, 
                   icn,
                   ww, hh,
                   0, 0, 
                   DI_NORMAL
                   );
      }
    }
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
  
  if(vwUseBufferedPaint)
  {
    vwBufferedPaintSetAlpha(hBufferedPaint, 0, 200);
    vwEndBufferedPaint(hBufferedPaint, TRUE);
  }
  else
  {
    BitBlt(ohdc, 0, 0, dcr.right - dcr.left, dcr.bottom-dcr.top, hdc, 0, 0, SRCCOPY);
    DeleteObject(hBmp);
  }
  DeleteDC(hdc);

  EndPaint(canvasWindowHandle, &ps);

  return 0L;
}


//int tipinit = 0;

/*
* Main startup function
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{  
  HINSTANCE libHandle ; 
  MSG msg;
  
  if((libHandle = LoadLibrary(_T("UxTheme"))) != NULL)
  {
    vwIsThemeActive = (vwISTHEMEACTIVE) GetProcAddress(libHandle,"IsThemeActive") ;
    if(((vwBufferedPaintSetAlpha = (vwBUFFEREDPAINTSETALPHA) GetProcAddress(libHandle,"BufferedPaintSetAlpha")) != NULL) &&
       ((vwBeginBufferedPaint = (vwBEGINBUFFEREDPAINT) GetProcAddress(libHandle,"BeginBufferedPaint")) != NULL) &&
       ((vwEndBufferedPaint = (vwENDBUFFEREDPAINT) GetProcAddress(libHandle,"EndBufferedPaint")) != NULL) &&
       ((vwBufferedPaintUnInit = (vwBUFFEREDPAINTUNINIT) GetProcAddress(libHandle,"BufferedPaintUnInit")) != NULL))
      vwBufferedPaintInit = (vwBUFFEREDPAINTINIT) GetProcAddress(libHandle,"BufferedPaintInit") ;
  }

  mouseHookRegister();

  mainWindowCreate(hInstance);
  createBrushes();

  //while(canvasWindowCreate() == 0)
 // {
  //  Sleep(1000);
 // }
  
  //tooltipCreate(canvasWindowHandle);

  /* main messge loop */
  try
  {
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  catch(std::runtime_error e)
  {
    MessageBox(mainWindowHandle, e.what(), "runtime error", 1);
  }

  deleteBrushes();
  if(vwUseBufferedPaint)
    vwBufferedPaintUnInit() ;

  return (int)msg.wParam;
}
