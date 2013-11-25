# Makefile for Audio-CD-Creator Data-CD-Creator

include ../make.inc

CC           = icc
LIB_DIR      = $d:/emx/lib 
CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol- /Q+ $(VACDEBUG)
INC	=	..\include
DEFS         = 
PROGS        = 
ODIR		= ./obj
CPPDIR	= .
BINDIR		=	../../dist
RES		=	..\res


OBJECTS	=  audiofolder.obj audiooverriddenwpmethods.obj \
			audioDialogProcs.obj audiosettingsDialogProcs.obj audiooverriddenwpclsmethods.obj
#audio_thread_and_object_procs.obj \

all:	
#$(OBJECTS)
	cd ../creator && make

audiofolder.obj:		audiofolder.cpp $(INC)\audiofolder.hh
	icc $(CPPFLAGS) /C+ -I$(INC) $<

audiooverriddenwpmethods.obj:	audiooverriddenwpmethods.cpp $(INC)\audiofolderhelp.h $(INC)\audiofolder.hh
	icc $(CPPFLAGS) /C+ -I$(INC) $<

audiooverriddenwpclsmethods.obj:	audiooverriddenwpclsmethods.cpp $(INC)\audiofolderhelp.h \
					$(INC)\audiofolder.hh
	icc $(CPPFLAGS) /C+ -I$(INC) $<

audioDialogProcs.obj:	audioDialogProcs.cpp $(INC)\audiofolderhelp.h $(INC)\audiofolder.hh $(INC)\cddb.h \
			  $(INC)\audiofolderres.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<

audiosettingsDialogProcs.obj:	audiosettingsDialogProcs.cpp $(INC)\audiofolderhelp.h $(INC)\audiofolder.hh $(INC)\cddb.h \
			  $(INC)\audiofolderres.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<

audio_thread_and_object_procs.obj:	audio_thread_and_object_procs.cpp $(INC)\audiofolderres.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<
      		
clean:
	-rm *.obj

cleaner:
	-rm *.*~
	-rm *.flc


distribution:


	
