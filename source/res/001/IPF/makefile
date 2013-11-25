# Makefile for Audio-CD-Creator Data-CD-Creator

include ./../../../make.inc

CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol- /Q+ $(VACDEBUG)
                  
INC	=	../include
RESDIR	=	./$(LANGUAGE)
RESDIR2	=	./$(LANGUAGE)
AUDIODIR	=	.\..\audiocreator
DATADIR	=	.\..\datacreator
SETTINGSDIR	=	.\..\creatorsettings
HELPSOURCE	=	$(RESDIR2)/ipf



#all:	tutorial$(LANGUAGE).inf
all:
	ipfc -i tutorial$(LANGUAGE).ipf
	ipfc audiofolder$(LANGUAGE).ipf                		

tutorial$(LANGUAGE).inf:	
	ipfc -i tutorial$(LANGUAGE).ipf
	ipfc audiofolder$(LANGUAGE).ipf                		

cleaner:


distribution:
