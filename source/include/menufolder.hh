#ifndef _MENUFOLDER_HH
#define _MENUFOLDER_HH


#pragma SOMAsDefault(on)
#include <som.hh>
#include <somcls.hh>
#pragma SOMNoMangling(on)
#include <wpfolder.hh>
#include <wppgm.hh>

#pragma SOMAsDefault (off)
class CONTEXTITEM;

class CWMenuFolder;



//	#pragma SOMAsDefault(off)
	
class MENUFOLDERINFO
{
public:
 	SOMClass * somClass;
 	SOMClass * tempClass;
 	CWMenuFolder *thisPtr;
	PFNWP oldContainerProc;
	BOOL tempMenuEnabled;
};
#pragma SOMAsDefault(pop)


class M_CWMenuFolder:public M_WPFolder
{
#pragma SOMClassName(*,"M_CWMenuFolder")
#pragma SOMClassVersion(*,1,2)
#pragma SOMCallstyle(oidl)
public:
	virtual PSZ  wpclsQueryTitle();
	virtual	ULONG wpclsQueryIconData(PICONINFO pIconInfo);
  virtual	ULONG wpclsQueryIconDataN(PICONINFO pIconInfo, ULONG ulIconIndex);
	virtual ULONG wpclsQueryDefaultHelp(PULONG HelpPanelId,PSZ HelpLibrary);	
#pragma SOMReleaseOrder()
};

class CWMenuFolder:public WPFolder
{
#pragma SOMClassName(*,"CWMenuFolder")
#pragma SOMMetaClass(CWMenuFolder,"M_CWMenuFolder")
#pragma SOMClassVersion(CWMenuFolder,1,2)
#pragma SOMCallstyle(oidl)
private:
  CONTEXTITEM * ptrCItem;
  BOOL bMenuEnabled;
  ULONG ulEnable;
  ULONG ulLastMenuIDPrivate;
  virtual void mfPrivateFreeMenu(void);
public:
  virtual CONTEXTITEM * mfPrivateBuildMenu(CWMenuFolder * wpFolder, ULONG * ulLastMenuId);
  virtual void mfInsertMItems( HWND hwndMenu,ULONG iPosition);
  virtual void wpInitData();
  virtual MRESULT wpDragOver(HWND hwndCnr,PDRAGINFO pDragInfo);
  virtual MRESULT wpDrop(HWND hwndCnr,PDRAGINFO pDragInfo,PDRAGITEM pDragItem);
  virtual BOOL wpAddSettingsPages(HWND hwndNotebook);
  virtual BOOL wpRestoreState(ULONG ulReserved);
  virtual BOOL wpSaveState();	
  virtual BOOL wpSetupOnce(PSZ pSetupString);
  virtual BOOL wpSetup(PSZ pSetupString);
  virtual void wpCopiedFromTemplate();
  virtual BOOL wpDeleteFromContent(WPObject *object);
  void mfFreeMenu();
  virtual BOOL mfQueryMenuEnabled()	{return bMenuEnabled;};
  void mfSetMenuEnabled(BOOL bEnabled){bMenuEnabled=bEnabled;};
  virtual ULONG mfInsertMenuItems(HWND hwndMenu,ULONG iPosition, ULONG ulLastMenuId);
  virtual BOOL mfCheckMenuItems(WPObject *wpObject,ULONG ulMenuId);
  virtual ULONG mfAddClassChoosePage(HWND hwndNotebook);

#pragma SOMReleaseOrder(\
"mfPrivateBuildMenu",\
"mfInsertMItems",\
"mfFreeMenu",\
"mfQueryMenuEnabled",\
"mfSetMenuEnabled",\
"mfInsertMenuItems",\
"mfCheckMenuItems",\
"mfAddClassChoosePage",\
"mfPrivateFreeMenu")
};

#endif


























