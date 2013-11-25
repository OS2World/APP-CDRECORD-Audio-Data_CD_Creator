# Makefile for Audio-CD copy

include ../make.inc

CC           = gcc
LIB_DIR      = f:/emx/lib 
#CPPFLAGS     = -Ge- /Gm+ -G4 -Gl  /O+ -Oc+ -Ol
CPPFLAGS     = -O -Zomf -Zcrtdll -s -Wall
INC		= ..\include
DEFS         = 
PROGS        =  
ODIR		= .
CPPDIR	= .
HELPERDIR	=	..\launch_helper
RESDIR	=	..\res\$(LANGUAGE)
OBJECTS	=	$(HELPERDIR)\message.obj $(HELPERDIR)\helper.obj


all:	lnchmkis.exe cdr2lnch.exe prntsize.exe cdsize.exe msinfo.exe wrapper.exe\
	writeaud.exe onthefly.exe writeimg.exe mp3size.exe mp3decod.exe \
	scanbus.exe cover.exe waveinfo.exe daommio.exe daowrite.exe daoinfo.exe \
	encstat.exe dvdsize.exe dvdthefly.exe

encstat.exe: encstat.c
	gcc $(CPPFLAGS) -I$(INC) $< -Zlinker /PMTYPE:VIO

wrapper.exe: wrapper.cpp
	gcc $(CPPFLAGS) -I$(INC) $< -Zlinker /PMTYPE:VIO
#	icc -G4 -Gl -Rn /O+ -Oc+ -Ol $<		
	
cdr2lnch.exe: cdr2lnch.c $(INC)/audiofolder.h
	gcc $(CPPFLAGS) -I $(INC) -s $< -Zlinker /PMTYPE:VIO

onthefly.exe:	onthefly.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

dvdthefly.exe:	dvdthefly.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

msinfo.exe:	msinfo.c
	gcc $(CPPFLAGS) $< -Zlinker /PMTYPE:VIO

lnchmkis.exe:	lnchmkis.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

prntsize.exe:	prntsize.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO		

cdsize.exe:	cdsize.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

dvdsize.exe:	dvdsize.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

writeimg.exe:	writeimg.c
	gcc $(CPPFLAGS) -s -I$(INC) -s $< -Zlinker /PMTYPE:VIO

writeaud.exe:	writeaud.c
	gcc $(CPPFLAGS) -s -I$(INC) -s $< -Zlinker /PMTYPE:VIO

daowrite.exe:	daowrite.c
	gcc $(CPPFLAGS) -s -I$(INC) -s $< -Zlinker /PMTYPE:VIO

daoinfo.exe:	daoinfo.c
	gcc $(CPPFLAGS) -s -I$(INC) -los2me -s $< -Zlinker /PMTYPE:VIO

mp3size.exe:	mp3size.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -los2me -Zlinker /PMTYPE:VIO

mp3decod.exe:	mp3decod.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -los2me -Zlinker /PMTYPE:VIO

scanbus.exe:	scanbus.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

cover.exe:	cover.c
	gcc $(CPPFLAGS) -I$(INC) -s $< -Zlinker /PMTYPE:VIO

waveinfo.exe:	waveinfo.c
	gcc $(CPPFLAGS) -I$(INC) -los2me -s $< -Zlinker /PMTYPE:VIO

daommio.exe:	daommio.c
	gcc $(CPPFLAGS) -I$(INC) -los2me -s $< -Zlinker /PMTYPE:VIO

$(HELPERDIR)\message.obj:	$(HELPERDIR)\message.c 
	gcc -c -Zomf -I$(INC) $< 		

$(HELPERDIR)\helper.obj:	$(HELPERDIR)\helper.c
	gcc -c -Zomf -I$(INC) $< 		
	
#$(HELPERDIR)\progbars.obj:	$(HELPERDIR)\progbars.c
#	gcc -c -Zomf -I$(INC) $< 		

#$(HELPERDIR)\gpih.obj:	$(HELPERDIR)\gpih.c
#	gcc -c -Zomf -I$(INC) $< 		

clean:
	-rm *.exe
	-rm *.o
	-rm *.obj

cleaner:
	-rm *.*~
	-rm *.flc

distribution:
	-cp *.exe ../../dist/bin
	
