/**
* search for window with given class name
*/
HWND findWindowByClass(HWND parent, char* className)
{
  HWND p,p2;
  char cls[1024];

  if(parent == NULL)
    return NULL;

  p = GetTopWindow(parent);
  while(p)
  {
    GetClassName(p, cls, sizeof(cls));
    if(strcmp(cls, className) == 0)
      return p;

    p2 = findWindowByClass(p, className);
    if(p2)
      return p2;

    p = GetNextWindow(p, GW_HWNDNEXT);
  }

  return NULL;
}

/**
* Searches for a window at given cursor position
*/
HWND getWindowAt(int x, int y, int desk, HWND skipWindow)
{
  HWND h = GetForegroundWindow(); 
  POINT pt = {x, y};
  while(h)
  {
    int flag  = (int)SendMessage(vwHandle, VW_WINGETINFO, (WPARAM) h, NULL);
    if(vwWindowGetInfoDesk(flag) == desk)
    {
      if(h != skipWindow)
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