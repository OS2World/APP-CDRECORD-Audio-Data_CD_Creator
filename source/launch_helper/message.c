#define INCL_WIN

#include <os2.h>
#include <stdio.h>

void errorResource()
{

  WinMessageBox(HWND_DESKTOP,0,
                "The resource DLL which contains all the dialogs, graphics and messages cannot be loaded. \
Please check your installation. There must be a file CDFLDxxx.DLL in the installation directory of the \
CD-Creator package. xxx is the country code of your system e.g. 049 for Germany. If there is no \
support for your language there must be at least the file CDFLD001.DLL. The CD creation features \
are not avaiable!",
"Problem with Audio/Data-CD-Creator installation",12345,
                MB_OK|MB_MOVEABLE|MB_ERROR);

}

ULONG messageBox( char* text, ULONG ulTextID , LONG lSizeText,
                  char* title, ULONG ulTitleID, LONG lSizeTitle,
                  HMODULE hResource, HWND hwnd, ULONG ulFlags)
{

  if(!WinLoadString(WinQueryAnchorBlock(hwnd),hResource,ulTextID,lSizeText,text)) {
    errorResource();
    return MBID_ERROR;
  }
  if(!WinLoadString(WinQueryAnchorBlock(hwnd),hResource,ulTitleID,lSizeTitle,title)) {
    errorResource();
    return MBID_ERROR;
  }
  return WinMessageBox( HWND_DESKTOP, hwnd, text, title, 0UL, ulFlags );
}

void getMessage(char* text,ULONG ulID, LONG lSizeText, HMODULE hResource,HWND hwnd)
{
  if(!WinLoadString(WinQueryAnchorBlock(hwnd),hResource,ulID,lSizeText,text)) {
    sprintf(text,"");
  }
}

void pmUsage()
{
  WinMessageBox(HWND_DESKTOP,0,
                "This helper shouldn't be started by hand. It is called by the CD-Creator classes. \
If you didn't launch the helper by hand you may have found a bug. Please contact the author.",
                "Problem with Audio/Data-CD-Creator",12345,
                MB_OK|MB_MOVEABLE|MB_ERROR);

}

