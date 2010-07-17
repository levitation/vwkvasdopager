// Mouse hook. Called internally by win32 from inside GetMessage()
LRESULT CALLBACK mouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
  // Hook chain
  if(nCode < 0)
    return CallNextHookEx(0, nCode, wParam, lParam);

  // Check if the mouse wheel is used
  if(wParam == WM_MOUSEWHEEL)
  {
    MSLLHOOKSTRUCT *hookStruct = (MSLLHOOKSTRUCT*)lParam;
    POINT p = hookStruct->pt;

    if(WindowFromPoint(p) == canvasWindowHandle)
    {
      if(vwHandle)
      {
        // change desk depending on scroll direction
        if((short)HIWORD(hookStruct->mouseData) > 0)
          PostMessage(vwHandle, VW_CHANGEDESK, VW_STEPPREV, 0);
        else
          PostMessage(vwHandle, VW_CHANGEDESK, VW_STEPNEXT, 0);
      }
      return 1; // Stop event propagation
    }
  }
  return CallNextHookEx(0, nCode, wParam, lParam);
}

void mouseHookRegister()
{
  // Register global win32 hook.
  // Fortunatly WH_MOUSE_LL doesn't require DLL injection
  HINSTANCE hInstance = GetModuleHandle(0);
  SetWindowsHookEx(WH_MOUSE_LL, mouseHookCallback, hInstance, 0);
}

