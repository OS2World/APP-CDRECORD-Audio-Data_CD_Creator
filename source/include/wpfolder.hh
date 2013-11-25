#ifndef _DTS_HH_INCLUDED_wpfolder
#define _DTS_HH_INCLUDED_wpfolder
/* Start Class WPFolder */

// This file was generated by the IBM "DirectToSOM" emitter for C++ (V1.77)
// Generated at 11/02/96 09:11:43 EST
// This is class: WPFolder
// Mangled class name: WPFolder
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
class M_WPFileSystem;
     #pragma SOMAsDefault(pop)
     #pragma SOMAsDefault(on)
class M_WPFolder;
     #pragma SOMAsDefault(pop)
#include <wpfsys.hh>
#pragma SOMNonDTS(on)

/******************************************************/
      /* Folder flags */

      #define FOI_POPULATEDWITHALL      0x0001
      #define FOI_POPULATEDWITHFOLDERS  0x0002
      #define FOI_WORKAREA              0x0004
      #define FOI_CHANGEFONT            0x0008        /* anti-recursion flag */
      #define FOI_WAMINIMIZED           0x0020
      #define FOI_WASTARTONRESTORE      0x0040
      #define FOI_NOREFRESHVIEWS        0x0080
      #define FOI_ASYNCREFRESHONOPEN    0x0100
      #define FOI_TREEPOPULATED         0x0200
      #define FOI_POPULATEINPROGRESS    0x0400                          /*DORM*/
      #define FOI_REFRESHINPROGRESS     0x0800
      #define FOI_FIRSTPOPULATE         0x1000  /* folder has no iconposdata */ //LD62414
      #define FOI_WAMCRINPROGRESS       0x2000  /* Minimize, close, restore in progress ihs67096 */
      #define FOI_CNRBKGNDOLDFORMAT     0x4000  /* CnrBkgnd saved in old format   ihs77154 */
      #define FOI_CHANGEICONBGNDCOLOR   0x8000  /* Icon Text Background Color changing ihs79998 */
      #define FOI_CHANGEICONTEXTCOLOR   0x00010000 /* Icon Text Color changing    ihs89146 */
      #define FOI_DELETEINPROGRESS      0x00020000 /* To prevent wpFree from repopulating */

      /* Open views for the wpOpen() method */

      #define OPEN_TREE          101
      #define OPEN_DETAILS       102

      /* Folder save-restore keys */

      #define IDKEY_FDRCONTENTATTR     2900
      #define IDKEY_FDRTREEATTR        2901
      #define IDKEY_FDRCVLFONT         2902
      #define IDKEY_FDRCVNFONT         2903
      #define IDKEY_FDRCVIFONT         2904
      #define IDKEY_FDRTVLFONT         2905
      #define IDKEY_FDRTVNFONT         2906
      #define IDKEY_FDRDETAILSATTR     2907
      #define IDKEY_FDRDVFONT          2908
      #define IDKEY_FDRDETAILSCLASS    2909
      #define IDKEY_FDRICONPOS         2910
      #define IDKEY_FDRINVISCOLUMNS    2914
      #define IDKEY_FDRINCCLASS        2920
      #define IDKEY_FDRINCNAME         2921
      #define IDKEY_FDRFSYSSEARCHINFO  2922
      #define IDKEY_FILTERCONTENT      2923
      #define IDKEY_CNRBACKGROUND      2924
      #define IDKEY_FDRINCCRITERIA     2925
      #define IDKEY_FDRICONVIEWPOS     2926
      #define IDKEY_FDRSORTCLASS       2927
      #define IDKEY_FDRSORTATTRIBS     2928
      #define IDKEY_FDRSORTINFO        2929
      #define IDKEY_FDRSNEAKYCOUNT     2930                              /*SNK*/
      #define IDKEY_FDRLONGARRAY       2931
      #define IDKEY_FDRSTRARRAY        2932
      #define IDKEY_FDRCNRBACKGROUND   2933  // This key was only used in
                                             // internal drivers 8.117-8.139
      #define IDKEY_FDRBKGNDIMAGEFILE  2934
      #define IDKEY_FDRBACKGROUND      2935
      #define IDKEY_FDRSELFCLOSE       2936

      /* Miscellaneous */

      #define PPFONTSTRSIZE 20

      #define ICONPOSSIZE(pI) ( sizeof(ICONPOS) + strlen(pI->szIdentity) )
      #define PARTIAL_FILLFOLDER_SEM_TIMEOUT 5   /* avoid wasteful       */     //LD62414
                                                 /* SEM_INDEFEINTE_WAIT  */     //LD62414
      #define GETATTR(View) (View == OPEN_CONTENTS ? _ContentAttr : \ 
                             (View == OPEN_TREE ? _TreeAttr : _DetailsAttr))
      #define FOLDER_DEFATTRS (CV_ICON | CA_OWNERPAINTBACKGROUND     |  \ 
                               CA_MIXEDTARGETEMPH | CA_OWNERDRAW)
      #define FOLDER_DEFTREEATTRS (CV_ICON | CA_OWNERPAINTBACKGROUND |  \ 
                                   CV_TREE | CA_TREELINE | CA_OWNERDRAW)
      #define FOLDER_DEFDETAILS (CV_DETAIL | CA_OWNERPAINTBACKGROUND |  \ 
                       CV_MINI | CA_DETAILSVIEWTITLES | CA_MIXEDTARGETEMPH)
      #define FOLDER_DEFSORT NULL

      /* wpSearchFolder structures */
       typedef struct _SEARCH_INFO {
          M_WPObject *ClassToBeFound;
          BOOL   fSkipExtendedSearchCriteria;
          PVOID  pvoidExtendedCriteria;
       } SEARCH_INFO;
       typedef SEARCH_INFO *PSEARCH_INFO;



/*--------------Migration------------*/
#define WPFolder_classObj WPFolderClassData.classObject
#define _WPFolder WPFolder_classObj


/*
 * Declare the class data structure
 */
SOMEXTERN struct WPFolderClassDataStructure {
	SOMClass *classObject;
	somMToken wpSetFldrFlags;
	somMToken wpQueryFldrFlags;
	somMToken wpSetFldrFont;
	somMToken wpQueryFldrFont;
	somMToken wpSetNextIconPos;
	somMToken wpQueryNextIconPos;
	somMToken wpPopulate;
	somMToken wpAddToContent;
	somMToken wpQueryContent;
	somMToken wpAddFolderView1Page;
	somMToken wpAddFolderView2Page;
	somMToken wpAddFolderView3Page;
	somMToken wpAddFolderIncludePage;
	somMToken wpAddFolderSortPage;
	somMToken wpAddFolderBackgroundPage;
	somMToken wpInitIconPosData;
	somMToken wpFreeIconPosData;
	somMToken wpQueryIconPosition;
	somMToken wpStoreIconPosData;
	somMToken wpDeleteFromContent;
	somMToken wpSetFldrAttr;
	somMToken wpQueryFldrAttr;
	somMToken wpSetFldrSort;
	somMToken wpQueryFldrSort;
	somMToken wpRestoreFldrRunObjs;
	somMToken wpStoreFldrRunObjs;
	somMToken withdrawn30;
	somMToken withdrawn31;
	somMToken wpDeleteContents;
	somMToken wpQueryOpenFolders;
	somMToken wpFolderPrivate1;
	somMToken wpSearchFolder;
	somMToken wpSetFldrDetailsClass;
	somMToken wpQueryFldrDetailsClass;
	somMToken wpRefresh;
	somMToken wpSetDetailsColumnVisibility;
	somMToken wpIsDetailsColumnVisible;
	somMToken wpFolderPrivate2;
	somMToken wpFolderPrivate3;
	somMToken wpFolderPrivate4;
	somMToken wpFolderPrivate5;
	somMToken wpSetFldrSortClass;
	somMToken wpQueryFldrSortClass;
	somMToken wpSetSortAttribAvailable;
	somMToken wpIsSortAttribAvailable;
	somMToken wpFolderPrivate6;
	somMToken wpContainsFolders;
	somMToken wpFolderPrivate7;
	somMToken wpQueryIconViewPos;
	somMToken wpQueryDefaultView;
	somMToken wpAddFirstChild;
	somMToken wpFolderPrivate8;
	somMToken wpHideFldrRunObjs;
	somMToken wpFolderPrivate9;
	somMToken wpFolderPrivate10;
	somMToken wpFolderPrivate11;
	somMToken wpFolderPrivate12;
	somMToken wpFolderPrivate13;
	somMToken wpFolderPrivate14;
	somMToken wpFolderPrivate15;
	somMToken wpFolderPrivate16;
	somMToken wpFolderPrivate17;
	somMToken wpFolderPrivate18;
	somMToken wpFolderPrivate19;
	somMToken wpFolderPrivate20;
	somMToken wpFolderPrivate21;
	somMToken wpFolderPrivate22;
	somMToken wpFolderPrivate23;
	somMToken wpModifyFldrFlags;
	somMToken wpFolderPrivate24;
	somMToken wpFolderPrivate25;
	somMToken wpFolderPrivate26;
	somMToken wpFolderPrivate27;
	somMToken wpFolderPrivate28;
	somMToken wpQueryIconTextBackgroundColor;
	somMToken wpSetIconTextBackgroundColor;
	somMToken wpFolderPrivate29;
	somMToken wpFolderPrivate30;
	somMToken wpFolderPrivate31;
	somMToken wpQueryIconTextColor;
	somMToken wpSetIconTextColor;
	somMToken wpQueryIconTextVisibility;
	somMToken wpSetIconTextVisibility;
	somMToken wpAddFolderSelfClosePage;
	somMToken wpFolderPrivate32;
	somMToken wpFolderPrivate33;
	somMToken wpFolderPrivate34;
	somMToken wpFolderPrivate35;
	somMToken wpFolderPrivate36;
	somMToken wpFolderPrivate37;
	somMToken wpFolderPrivate38;
	somMToken wpFolderPrivate39;
	somMToken wpFolderPrivate40;
	somMToken wpFolderPrivate41;
	somMToken wpFolderPrivate42;
	somMToken wpAddFolderMenu2Page;
	somMToken wpSetMenuBarVisibility;
	somMToken wpQueryMenuBarVisibility;
	somMToken wpFolderPrivate43;
	somMToken wpFolderPrivate44;
	somMToken wpFolderPrivate45;
	somMToken wpFlushNotifications;
	somMToken wpSetShadowTextColor;
	somMToken wpQueryShadowTextColor;
	somMToken wpSetFldrBackground;
	somMToken wpQueryFldrBackground;
	somMToken wpSetFldrGrid;
	somMToken wpQueryFldrGrid;
} SOMDLINK WPFolderClassData;

/*
 * Declare the C specific class data structure
 */
SOMEXTERN struct WPFolderCClassDataStructure {
	somMethodTabs parentMtab;
	somDToken instanceDataToken;
} SOMDLINK WPFolderCClassData;


/*
 * Override method: wpSetup
 */
#define WPFolder_wpSetup(somSelf,pszSetupString) \
	WPObject_wpSetup(somSelf,pszSetupString)


/*
 * New Method: wpAddToContent
 */
typedef BOOL   SOMLINK somTP_WPFolder_wpAddToContent(WPFolder *somSelf,
		WPObject* Object);
//#pragma linkage(somTP_WPFolder_wpAddToContent, system)
typedef somTP_WPFolder_wpAddToContent *somTD_WPFolder_wpAddToContent;
#define somMD_WPFolder_wpAddToContent "::WPFolder::wpAddToContent"
#define WPFolder_wpAddToContent(somSelf,Object) \
    (SOM_Resolve(somSelf, WPFolder, wpAddToContent) \
	(somSelf,Object))
#ifndef SOMGD_wpAddToContent
    #if (defined(_wpAddToContent) || defined(__wpAddToContent))
        #undef _wpAddToContent
        #undef __wpAddToContent
        #define SOMGD_wpAddToContent 1
    #else
        #define _wpAddToContent WPFolder_wpAddToContent
    #endif /* _wpAddToContent */
#endif /* SOMGD_wpAddToContent */


/*
 * New Method: wpDeleteFromContent
 */
typedef BOOL   SOMLINK somTP_WPFolder_wpDeleteFromContent(WPFolder *somSelf,
		WPObject* Object);
//#pragma linkage(somTP_WPFolder_wpDeleteFromContent, system)
typedef somTP_WPFolder_wpDeleteFromContent *somTD_WPFolder_wpDeleteFromContent;
#define somMD_WPFolder_wpDeleteFromContent "::WPFolder::wpDeleteFromContent"
#define WPFolder_wpDeleteFromContent(somSelf,Object) \
    (SOM_Resolve(somSelf, WPFolder, wpDeleteFromContent) \
	(somSelf,Object))
#ifndef SOMGD_wpDeleteFromContent
    #if (defined(_wpDeleteFromContent) || defined(__wpDeleteFromContent))
        #undef _wpDeleteFromContent
        #undef __wpDeleteFromContent
        #define SOMGD_wpDeleteFromContent 1
    #else
        #define _wpDeleteFromContent WPFolder_wpDeleteFromContent
    #endif /* _wpDeleteFromContent */
#endif /* SOMGD_wpDeleteFromContent */



/******************************************************/




#pragma SOMAsDefault(on)
class WPFolder : public WPFileSystem {

	#pragma SOMClassName(*, "WPFolder")
	#pragma SOMNoMangling(*)
	#pragma SOMNonDTS(*)

     #pragma SOMClassName (WPFolder, "WPFolder")
//     #pragma SOMMetaClass (WPFolder, "M_WPFolder")
     #pragma SOMClassVersion (WPFolder, 1, 2)
     #pragma SOMCallstyle (oidl)
     #pragma SOMAsDefault(off)
     #pragma SOMAsDefault(pop)
  public :
     virtual BOOL wpSetFldrFlags(ULONG ulFlags);
     virtual ULONG wpQueryFldrFlags();
     virtual BOOL wpSetFldrFont(PSZ pszFont, ULONG ulView);
     virtual PSZ wpQueryFldrFont(ULONG ulView);
     virtual BOOL wpSetFldrAttr(ULONG Attr, ULONG ulView);
     virtual ULONG wpQueryFldrAttr(ULONG ulView);
     virtual BOOL wpSetNextIconPos(PPOINTL pptl);
     virtual PPOINTL wpQueryNextIconPos();
     virtual BOOL wpPopulate(ULONG ulReserved, PSZ pszPath, BOOL fFoldersOnly);
     virtual WPObject* wpQueryContent(WPObject* Object, ULONG ulOption);
     virtual ULONG wpAddFolderView1Page(HWND hwndNotebook);
     virtual ULONG wpAddFolderView2Page(HWND hwndNotebook);
     virtual ULONG wpAddFolderView3Page(HWND hwndNotebook);
     virtual ULONG wpAddFolderIncludePage(HWND hwndNotebook);
     virtual ULONG wpAddFolderSortPage(HWND hwndNotebook);
     virtual ULONG wpAddFolderBackgroundPage(HWND hwndNotebook);
     virtual ULONG wpAddFolderSelfClosePage(HWND hwndNotebook);
     virtual BOOL wpInitIconPosData();
     virtual void wpFreeIconPosData();
     virtual BOOL wpStoreIconPosData(PICONPOS pIconPos, ULONG cbSize);
     virtual BOOL wpQueryIconPosition(PSZ pszIdentity, PPOINTL pptl, 
                                      PULONG pIndex);
     virtual BOOL wpSetFldrSort(PVOID pSortRecord, ULONG ulView, 
                                ULONG ulType);
     virtual PVOID wpQueryFldrSort(ULONG ulView, ULONG ulType);
     virtual BOOL wpRestoreFldrRunObjs();
     virtual BOOL wpStoreFldrRunObjs(ULONG ulType);
     virtual BOOL wpHideFldrRunObjs(BOOL fHide);
     virtual ULONG wpDeleteContents(ULONG fConfirmations);
     virtual BOOL wpSetFldrDetailsClass(M_WPObject* Class);
     virtual M_WPObject* wpQueryFldrDetailsClass();
     virtual BOOL wpSearchFolder(PSZ pszName, ULONG ulSearchType, 
                                 ULONG ulLen, PSEARCH_INFO pInfo, 
                                 WPFolder* ResultFolder);
     virtual BOOL wpContainsFolders(BOOL* pfSubFolders);
     virtual WPObject* wpQueryOpenFolders(ULONG ulOption);
     virtual BOOL wpModifyFldrFlags(ULONG ulFlags, ULONG ulFlagMask);
     virtual BOOL wpAddToContent(WPObject* Object);
     virtual BOOL wpDeleteFromContent(WPObject* Object);
     virtual BOOL wpSetDetailsColumnVisibility(ULONG index, BOOL Visible);
     virtual BOOL wpIsDetailsColumnVisible(ULONG index);
     virtual BOOL wpSetFldrSortClass(M_WPObject* Class);
     virtual M_WPObject* wpQueryFldrSortClass();
     virtual BOOL wpSetSortAttribAvailable(ULONG index, BOOL Available);
     virtual BOOL wpIsSortAttribAvailable(ULONG index);
     virtual char* wpQueryIconViewPos();
     virtual WPObject* wpAddFirstChild();
     virtual void wpInitData();
     virtual void wpUnInitData();
     virtual BOOL wpFree();
     virtual ULONG wpDelete(ULONG fConfirmations);
     virtual ULONG wpConfirmDelete(ULONG fConfirmations);
     virtual BOOL wpSaveState();
     virtual BOOL wpRestoreState(ULONG ulReserved);
     virtual BOOL wpMenuItemSelected(HWND hwndFrame, ULONG ulMenuId);
     virtual BOOL wpModifyPopupMenu(HWND hwndMenu, HWND hwndCnr, 
                                    ULONG iPosition);
     virtual BOOL wpAddSettingsPages(HWND hwndNotebook);
     virtual HWND wpOpen(HWND hwndCnr, ULONG ulView, ULONG param);
     virtual BOOL wpSetup(PSZ pszSetupString);
     virtual BOOL wpMoveObject(WPFolder* Folder);
     virtual MRESULT wpDrop(HWND hwndCnr, PDRAGINFO pdrgInfo, 
                            PDRAGITEM pdrgItem);
     virtual MRESULT wpDragOver(HWND hwndCnr, PDRAGINFO pdrgInfo);
     virtual BOOL wpMenuItemHelpSelected(ULONG MenuId);
     virtual ULONG wpAddFile3Page(HWND hwndNotebook);
     virtual ULONG wpAddFile2Page(HWND hwndNotebook);
     virtual BOOL wpFormatDragItem(PDRAGITEM pdrgItem);
     virtual MRESULT wpRender(PDRAGTRANSFER pdxfer);
     virtual BOOL wpRefresh(ULONG ulView, PVOID pReserved);
     virtual ULONG wpFilterPopupMenu(ULONG ulFlags, HWND hwndCnr, 
                                     BOOL fMultiSelect);
     virtual ULONG wpQueryDefaultView();
     virtual MRESULT wpRenderComplete(PDRAGTRANSFER pdxfer, ULONG ulResult);
     virtual BOOL wpQueryDefaultHelp(PULONG pHelpPanelId, PSZ HelpLibrary);
     virtual void wpObjectReady(ULONG ulCode, WPObject* refObject);
     virtual ULONG wpAddObjectWindowPage(HWND hwndNotebook);
     virtual BOOL wpSetIconData(PICONINFO pIconInfo);
     #pragma SOMReleaseOrder ( \
			      "wpSetFldrFlags", \
			      "wpQueryFldrFlags", \
			      "wpSetFldrFont", \
			      "wpQueryFldrFont", \
			      "wpSetNextIconPos", \
			      "wpQueryNextIconPos", \
			      "wpPopulate", \
			      "wpAddToContent", \
			      "wpQueryContent", \
			      "wpAddFolderView1Page", \
			      "wpAddFolderView2Page", \
			      "wpAddFolderView3Page", \
			      "wpAddFolderIncludePage", \
			      "wpAddFolderSortPage", \
			      "wpAddFolderBackgroundPage", \
			      "wpInitIconPosData", \
			      "wpFreeIconPosData", \
			      "wpQueryIconPosition", \
			      "wpStoreIconPosData", \
			      "wpDeleteFromContent", \
			      "wpSetFldrAttr", \
			      "wpQueryFldrAttr", \
			      "wpSetFldrSort", \
			      "wpQueryFldrSort", \
			      "wpRestoreFldrRunObjs", \
			      "wpStoreFldrRunObjs", \
			      *, \
			      *, \
			      "wpDeleteContents", \
			      "wpQueryOpenFolders", \
			      *, \
			      "wpSearchFolder", \
			      "wpSetFldrDetailsClass", \
			      "wpQueryFldrDetailsClass", \
						"wpSetDetailsColumnVisibility", \
			      "wpIsDetailsColumnVisible", \
			      *, \
			      *, \
			      *, \
			      *, \
			      "wpSetFldrSortClass", \
			      "wpQueryFldrSortClass", \
			      "wpSetSortAttribAvailable", \
			      "wpIsSortAttribAvailable", \
			      *, \
			      "wpContainsFolders", \
			      *, \
			      "wpQueryIconViewPos", \
						"wpAddFirstChild", \
			      *, \
			      "wpHideFldrRunObjs", \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      "wpModifyFldrFlags", \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      "wpAddFolderSelfClosePage", \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *, \
			      *)
};
#pragma SOMAsDefault(pop)

     #pragma SOMNonDTS(pop)
/* End WPFolder */




/* Start Class M_WPFolder */

#include <wpfsys.hh>
#include <somcls.hh>
#pragma SOMNonDTS(on)

#pragma SOMAsDefault(on)

/*
 * Declare the class data structure
 */
SOMEXTERN struct M_WPFolderClassDataStructure {
	SOMClass *classObject;
	somMToken wpclsFolderPrivate1;
	somMToken wpclsFolderPrivate2;
	somMToken wpclsQueryOpenFolders;
	somMToken wpclsFolderPrivate3;
	somMToken wpclsQueryIconDataN;
	somMToken wpclsQueryIconN;
} SOMDLINK M_WPFolderClassData;

/*
 * Declare the C specific class data structure
 */
SOMEXTERN struct M_WPFolderCClassDataStructure {
	somMethodTabs parentMtab;
	somDToken instanceDataToken;
} SOMDLINK M_WPFolderCClassData;


/*
 * New Method: wpclsQueryOpenFolders
 */
typedef WPFolder*   SOMLINK somTP_M_WPFolder_wpclsQueryOpenFolders(M_WPFolder *somSelf, 
		WPFolder* Folder, 
		ULONG ulOption, 
		BOOL fLock);
//#pragma linkage(somTP_M_WPFolder_wpclsQueryOpenFolders, system)
typedef somTP_M_WPFolder_wpclsQueryOpenFolders *somTD_M_WPFolder_wpclsQueryOpenFolders;
#define somMD_M_WPFolder_wpclsQueryOpenFolders "::M_WPFolder::wpclsQueryOpenFolders"
#define M_WPFolder_wpclsQueryOpenFolders(somSelf,Folder,ulOption,fLock) \
    (SOM_Resolve(somSelf, M_WPFolder, wpclsQueryOpenFolders) \
	(somSelf,Folder,ulOption,fLock))
#ifndef SOMGD_wpclsQueryOpenFolders
    #if (defined(_wpclsQueryOpenFolders) || defined(__wpclsQueryOpenFolders))
        #undef _wpclsQueryOpenFolders
        #undef __wpclsQueryOpenFolders
        #define SOMGD_wpclsQueryOpenFolders 1
    #else
        #define _wpclsQueryOpenFolders M_WPFolder_wpclsQueryOpenFolders
    #endif /* _wpclsQueryOpenFolders */
#endif /* SOMGD_wpclsQueryOpenFolders */

class M_WPFolder : public M_WPFileSystem {

	#pragma SOMClassName(*, "M_WPFolder")
	#pragma SOMNoMangling(*)
	#pragma SOMNonDTS(*)

     #pragma SOMClassVersion (M_WPFolder, 1, 2)
     #pragma SOMCallstyle (oidl)
     #pragma SOMAsDefault(off)
     #pragma SOMAsDefault(pop)
  public :
     virtual ULONG wpclsQueryIconDataN(ICONINFO* pIconInfo, ULONG ulIconIndex);
     virtual HPOINTER wpclsQueryIconN(ULONG ulIconIndex);
     virtual WPFolder* wpclsQueryOpenFolders(WPFolder* Folder, 
                                             ULONG ulOption, 
                                             BOOL fLock);
     virtual void wpclsInitData();
     virtual ULONG wpclsQueryDefaultView();
     virtual PSZ wpclsQueryTitle();
     virtual ULONG wpclsQueryIconData(PICONINFO pIconInfo);
     virtual ULONG wpclsQueryStyle();
     virtual BOOL wpclsQueryDefaultHelp(PULONG pHelpPanelId, 
                                        PSZ pszHelpLibrary);
     #pragma SOMReleaseOrder ( \
			      *, \
			      *, \
			      "wpclsQueryOpenFolders", \
			      *, \
			      "wpclsQueryIconDataN", \
			      "wpclsQueryIconN")
};
     #pragma SOMMetaClass (WPFolder, "M_WPFolder")

#pragma SOMAsDefault(pop)

     #pragma SOMNonDTS(pop)
/* End M_WPFolder */
#endif /* _DTS_HH_INCLUDED_wpfolder */
