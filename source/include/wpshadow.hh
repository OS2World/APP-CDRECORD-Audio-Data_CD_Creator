#ifndef _DTS_HH_INCLUDED_wpshadow
#define _DTS_HH_INCLUDED_wpshadow
/* Start Class WPShadow */

// This file was generated by the IBM "DirectToSOM" emitter for C++ (V1.77)
// Generated at 10/15/98 07:23:24 EDT
// This is class: WPShadow
// Mangled class name: WPShadow
// SE: May 19/94

#include <som.hh>

     #pragma SOMAsDefault(on)
class SOMClass;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class SOMObject;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class WPFolder;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class M_WPObject;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class WPObject;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class M_WPAbstract;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class M_WPShadow;
     #pragma SOMAsDefault(pop)
#include <wpabs.hh>
#pragma SOMNonDTS(on)

#pragma SOMAsDefault(on)
class WPShadow : public WPAbstract {

	#pragma SOMClassName(*, "WPShadow")
	#pragma SOMNoMangling(*)
	#pragma SOMNonDTS(*)

     #pragma SOMClassName (WPShadow, "WPShadow")
	// #pragma SOMMetaClass (WPShadow, "M_WPShadow")
     #pragma SOMClassVersion (WPShadow, 1, 2)
     #pragma SOMCallstyle (oidl)
     #pragma SOMAsDefault(off)
     #pragma SOMAsDefault(pop)
  public :
     virtual WPObject* wpQueryShadowedObject(BOOL fLock);
     virtual BOOL wpSetShadowTitle(PSZ pszNewTitle);
     virtual BOOL wpSetLinkToObject(WPObject* FromObject);
     virtual PSZ wpQueryTitle();
     virtual BOOL wpSetup(PSZ pszSetupString);
     virtual BOOL wpSaveState();
     virtual BOOL wpRestoreState(ULONG ulReserved);
     virtual ULONG wpQueryStyle();
     virtual void wpInitData();
     virtual void wpUnInitData();
     virtual BOOL wpSetTitle(PSZ pszNewTitle);
     virtual ULONG wpFilterPopupMenu(ULONG ulFlags, HWND hwndCnr, 
                                     BOOL fMultiSelect);
     virtual BOOL wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, 
                                    ULONG iPosition);
     virtual BOOL wpMenuItemSelected(HWND hwndFrame, ULONG ulMenuId);
     virtual HWND wpViewObject(HWND hwndCnr, ULONG ulView, ULONG param);
     virtual BOOL wpMenuItemHelpSelected(ULONG MenuId);
     virtual WPObject* wpCreateFromTemplate(WPFolder* folder, 
                                            BOOL fLock);
     virtual HWND wpOpen(HWND hwndCnr, ULONG ulView, ULONG param);
     virtual BOOL wpInsertPopupMenuItems(HWND hwndMenu, ULONG iPosition, 
                                         HMODULE hmod, ULONG MenuID, 
                                         ULONG SubMenuID);
     virtual WPObject* wpCreateShadowObject(WPFolder* Folder, 
                                            BOOL fLock);
     virtual MRESULT wpDragOver(HWND hwndCnr, PDRAGINFO pdrgInfo);
     virtual MRESULT wpDrop(HWND hwndCnr, PDRAGINFO pdrgInfo, 
                            PDRAGITEM pdrgItem);
     virtual BOOL wpQueryDefaultHelp(PULONG pHelpPanelId, PSZ HelpLibrary);
     virtual ULONG wpConfirmDelete(ULONG fConfirmations);
     virtual ULONG wpConfirmObjectTitle(WPFolder* Folder, WPObject** ppDuplicate, 
                                        PSZ pszTitle, ULONG cbTitle, 
                                        ULONG menuID);
     virtual BOOL wpPrintObject(PPRINTDEST pPrintDest, ULONG ulReserved);
     virtual BOOL wpFormatDragItem(PDRAGITEM pdrgItem);
     virtual MRESULT wpDraggedOverObject(WPObject* DraggedOverObject);
     virtual BOOL wpDroppedOnObject(WPObject* DroppedOnObject);
     virtual ULONG wpQueryNameClashOptions(ULONG menuID);
     #pragma SOMReleaseOrder ( \
			      "wpSetLinkToObject", \
			      *, \
			      "wpSetShadowTitle", \
			      "wpQueryShadowedObject")
};
#pragma SOMAsDefault(pop)

     #pragma SOMNonDTS(pop)
/* End WPShadow */
/* Start Class M_WPShadow */

#include <wpabs.hh>
#include <somcls.hh>
#pragma SOMNonDTS(on)

#pragma SOMAsDefault(on)
class M_WPShadow : public M_WPAbstract {

	#pragma SOMClassName(*, "M_WPShadow")
	#pragma SOMNoMangling(*)
	#pragma SOMNonDTS(*)

     #pragma SOMClassVersion (M_WPShadow, 1, 2)
     #pragma SOMCallstyle (oidl)
     #pragma SOMAsDefault(off)
     #pragma SOMAsDefault(pop)
  public :
     virtual PSZ wpclsQueryTitle();
     virtual ULONG wpclsQueryStyle();
     virtual ULONG wpclsQueryIconData(PICONINFO pIconInfo);
     #pragma SOMReleaseOrder ()
};
#pragma SOMAsDefault(pop)

     #pragma SOMNonDTS(pop)
/* End M_WPShadow */
#endif /* _DTS_HH_INCLUDED_wpshadow */
