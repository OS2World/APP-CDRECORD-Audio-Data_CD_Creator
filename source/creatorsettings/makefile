# Makefile for Audio-CD-Creator Data-CD-Creator

include ../make.inc

CC           = icc
LIB_DIR      = $d:/emx/lib 
CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol /Q+ $(VACDEBUG)
INC	=	..\include
DEFS         = 
PROGS        = 
OBJDIR		= 	.
CDIR		=	.
CPPDIR		=	 .
BINDIR		=	../../dist
RES		=	..\res


OBJECTS	=  	creatorsettings.obj settingsoverriddenwpmethods.obj \
			settingsoverriddenwpclsmethods.obj settingsdialogprocs.obj \
			cdrdaosettingsdialogprocs.obj mp3settingsdialogprocs.obj

all:	
#$(OBJECTS)
	cd ..\creator && make

###################################
#
#    Rules for compiling
#
###################################

$(OBJDIR)/%.obj:	$(CPPDIR)/%.cpp $(INC)\audiofolderhelp.h \
			$(INC)\creatorsettings.hh $(INC)\audiofolderres.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<

settingsoverriddenwpmethods.obj:	settingsoverriddenwpmethods.cpp $(INC)\audiofolderhelp.h $(INC)\creatorsettings.hh $(INC)\cddb.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<


clean:
	-rm *.obj

cleaner:
	-rm *.*~
	-rm *.flc


distribution:


	
