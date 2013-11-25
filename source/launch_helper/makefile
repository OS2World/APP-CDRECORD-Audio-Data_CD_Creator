# Makefile for Audio-CD copy

include ../make.inc

CC           = gcc
LIB_DIR      = $d:/emx/lib 
#CPPFLAGS     = -Ge- /Gm+ -G4 -Gl  /O+ -Oc+ -Ol
CPPFLAGS     = -O -Zomf -Zcrtdll -s
INC		= ../include
DEFS         = 
PROGS        =  
ODIR		= .
CPPDIR	= .
HELPERDIR	=	..\launch_helper
RESDIR	=	..\res\$(LANGUAGE)
OBJECTS	=	$(HELPERDIR)\message.obj $(HELPERDIR)\helper.obj

.PHONY:	clean cleaner

all:
	cd ..\. && make

clean:
	-rm *.exe
	-rm *.o
	-rm *.obj

cleaner:
	-rm *.*~
	-rm *.flc

	
