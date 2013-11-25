# Makefile for Audio-CD-Creator Data-CD-Creator

include ../make.inc

CC           = icc
LIB_DIR      = $d:/emx/lib
#CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol /Q+ $(VACDEBUG)
CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol- /Q+ $(VACDEBUG)
                  
INC	=	..\include
ALLINC	=	$(INC)\audiofolder.h $(INC)\audiofolder.hh $(INC)\audiofolderres.h 
DEFS         = 
PROGS        = 
ODIR		= 	./obj
CPPDIR		=	.
BINDIR		=	../../dist
DLLDIR		=	../../dist
RESDIR		=	..\res\$(LANGUAGE)
RESBASE2	=	../res
RESDIR2		=	../res/$(LANGUAGE)
RESOBJ		=	../res/obj
CREATORDIR	=	./
AUDIODIR	=	../audiocreator
DATADIR	=	../datacreator
DVDDIR		=	../dvdcreator
SETTINGSDIR	=	../creatorsettings
HELPERDIR	=	../helper
HELPSOURCE	=	$(RESDIR2)/ipf


# Audio folder
AUDIOOBJECTS	=	$(AUDIODIR)/audiofolder.obj \
			$(AUDIODIR)/audiooverriddenwpmethods.obj \
			$(AUDIODIR)/audiodialogprocs.obj \
			$(AUDIODIR)/audiooverriddenwpclsmethods.obj

# Settings
SETTINGSOBJECTS	=	$(SETTINGSDIR)/settingsoverriddenwpmethods.obj \
				$(SETTINGSDIR)/settingsoverriddenwpclsmethods.obj \
				$(SETTINGSDIR)/creatorsettings.obj \
				$(SETTINGSDIR)/settingsdialogprocs.obj \
				$(SETTINGSDIR)/mp3settingsdialogprocs.obj \
				$(SETTINGSDIR)/cdrdaosettingsdialogprocs.obj

#Data folder
DATAOBJECTS		=	$(DATADIR)/dataoverriddenwpmethods.obj \
				$(DATADIR)/dataoverriddenwpclsmethods.obj \
				$(DATADIR)/datadialogprocs.obj  \
				$(DATADIR)/datasettingsprocs.obj \
				$(DATADIR)/datacdfolder.obj

DVDOBJECTS		=	$(DVDDIR)/dvdoverriddenwpclsmethods.obj \
				$(DVDDIR)/dvdoverriddenwpmethods.obj \
				$(DVDDIR)/dvddialogprocs.obj

HELPEROBJECTS	=	$(HELPERDIR)/fillbar.obj \
				$(HELPERDIR)/pmhelper_customprocs.obj \
				$(HELPERDIR)/cd.obj \
				$(HELPERDIR)/messagehelper.obj \
				$(HELPERDIR)/helper.obj \
				$(HELPERDIR)/logfilehelper.obj \
				$(HELPERDIR)/somandwpshelper.obj \
				$(HELPERDIR)/except.obj 


OBJECTS		= 	$(AUDIOOBJECTS) \
				$(DATAOBJECTS) \
				$(SETTINGSOBJECTS) \
				$(HELPEROBJECTS) \
				$(DVDOBJECTS) \
				$(CREATORDIR)/sendconfig.obj \
				$(CREATORDIR)/cdtoolsdlgproc.obj \
				$(CREATORDIR)/threads_and_objectprocs.obj \
				$(CREATORDIR)/launchandwrap.obj \
				$(CREATORDIR)/launchpad.obj


all:	audiofolder.dll \
	$(RESOBJ)\cdfld$(LANGUAGE).dll \
	$(HELPSOURCE)\tutorial$(LANGUAGE).inf \
	$(HELPSOURCE)\audiofolder$(LANGUAGE).hlp


audiofolder.dll:	$(OBJECTS) \
		..\ProgFolder\progfldr.lib \
		$(RESOBJ)\audiofolder$(LANGUAGE).res \
		creator.def \
		$(RESOBJ)\audiofolder.res
		@echo "[33;1;m   Linking "$@"...[0;m"
		@icc -Ge- /Q+ $(OBJECTS) ..\ProgFolder\progfldr.lib MMPM2.lib creator.def >NUL
		@echo "[33;1;m   Adding resources to "$@"...[0;m"
		@rc $(RESOBJ)\audiofolder.res audiofolder.dll >NUL
		@echo "[32;1;m"$@" Done[0;m"
#		MAPSYM audiofolder.dll

$(RESOBJ)\audiofolder.res:		$(RESBASE2)\base\audiofolder.rc
		@cd $(RESBASE2) && make 

$(RESOBJ)\audiofolder$(LANGUAGE).res:	 \
		$(RESDIR)\audiofolder$(LANGUAGE).rc \
		$(RESDIR)\audiofolder$(LANGUAGE).dlg \
		$(INC)\audiofolderres.h
		@cd $(RESBASE2) && make 

$(RESOBJ)\cdfld$(LANGUAGE).dll:
		cd $(RESBASE2) && make

$(HELPSOURCE)\audiofolder$(LANGUAGE).hlp:	$(HELPSOURCE)\audiofolder$(LANGUAGE).ipf
		cd $(HELPSOURCE) && make

$(HELPSOURCE)\tutorial$(LANGUAGE).inf: $(HELPSOURCE)\tutorial$(LANGUAGE).ipf
		cd $(HELPSOURCE) && make tutorial$(LANGUAGE).inf


#######################################
# Compile command for the audio folder files
#######################################
$(AUDIODIR)/%.obj:	$(AUDIODIR)/%.cpp $(INC)/audiofolderhelp.h $(ALLINC)
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<

$(AUDIODIR)\audiodialogprocs.obj:	$(AUDIODIR)\audioDialogProcs.cpp $(INC)\audiofolderhelp.h $(ALLINC) $(INC)\cddb.h
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<


#######################################
#  Compile command for the data folder files
#######################################
$(DATADIR)/%.obj:	$(DATADIR)/%.cpp $(INC)\audiofolderres.h $(ALLINC)
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<

#######################################
#  Compile command for the DVD folder files
#######################################
$(DVDDIR)/%.obj:	$(DVDDIR)/%.cpp $(INC)\audiofolderres.h $(ALLINC)
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<

#######################################
# Compile command for the settings object files
#######################################
$(SETTINGSDIR)\settingsoverriddenwpmethods.obj:	$(SETTINGSDIR)/settingsoverriddenwpmethods.cpp $(INC)\audiofolderhelp.h \
							$(INC)\creatorsettings.hh $(INC)\cddb.h
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC)  /Fo$@ $<

$(SETTINGSDIR)/%.obj:	$(SETTINGSDIR)/%.cpp $(INC)\audiofolderhelp.h $(INC)\creatorsettings.hh $(INC)\audiofolderres.h
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC)  /Fo$@ $<


#######################################
# Compile command for the helper files
#######################################
$(HELPERDIR)/cd.obj:	$(HELPERDIR)/cd.cpp $(INC)\cddb.h
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<

$(HELPERDIR)/except.obj:	$(HELPERDIR)/except.cpp
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /Sm /C+ -I$(INC) /Fo$@ $<

$(HELPERDIR)/%.obj:	$(HELPERDIR)/%.cpp
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) /Fo$@ $<


#######################################
# Compile command for the misc files
#######################################
$(CREATORDIR)/%.obj:	%.cpp $(ALLINC)
	@echo "Compiling "$<
	@icc $(CPPFLAGS) /C+ -I$(INC) $<

clean:
	-rm *.sym
	-rm *.obj
	-rm $(RESDIR)\*.ico
	-rm *.dll
	-rm $(RESDIR2)/*.res
	-rm $(RESDIR2)/ipf/*.hlp
	-rm $(RESDIR2)/ipf/*.inf
	-cd ../audiocreator && make clean
	-cd ../datacreator && make clean
	-cd ../creatorsettings && make clean
	-cd $(RESBASE2) && make clean


cleaner:
	-rm *.*~
	-rm *.flc
	-cd ../audiocreator && make cleaner
	-cd ../datacreator && make cleaner
	-cd ../creatorsettings && make cleaner
	-rm $(HELPSOURCE)/*.*~
	-rm $(RESDIR2)/*.flc

distribution:
	-cd $(RESBASE2) && make distribution
	-cp -f audiofolder.dll $(DLLDIR)/aucdfldr.dll
	-cp -f $(HELPSOURCE)/audiofolder$(LANGUAGE).hlp $(BINDIR)/Help/aucdfldr.hlp
	-cp -f $(RESOBJ)/cdfld$(LANGUAGE).dll $(DLLDIR)/cdfld$(LANGUAGE).dll
	-cp -f $(HELPSOURCE)/tutorial$(LANGUAGE).inf $(BINDIR)/Help/tutorial.inf

	
