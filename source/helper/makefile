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


OBJECTS	=   cd.obj pmhelper_customprocs.obj messagehelper.obj helper.obj\
	somandwpshelper.obj logfilehelper.obj except.obj fillbar.obj

all:	drusrctl.dll
	cd ../creator && make

gpih.obj:	gpih.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

fillbar.obj:	fillbar.cpp 
	icc $(CPPFLAGS) /C+ -I$(INC) $<

cd.obj:		cd.cpp $(INC)\cddb.h
	icc $(CPPFLAGS) /C+ -I$(INC) $<
 
messagehelper.obj:	messagehelper.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

helper.obj:	helper.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

logfilehelper.obj:	logfilehelper.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

except.obj:	except.cpp
	icc $(CPPFLAGS) /Sm /C+ -I$(INC) $<

debug.obj:	debug.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<
      	
pmhelper_customprocs.obj:	pmhelper_customprocs.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

somandwpshelper.obj:	somandwpshelper.cpp
	icc $(CPPFLAGS) /C+ -I$(INC) $<

drusrctl.dll:	drusrctl.obj  drusrctl.def
	ILINK /NOFREE drusrctl.obj,drusrctl.dll,,REXX,drusrctl.def;


drusrctl.obj:	drusrctl.c
	icc -c -Ge- drusrctl.c
	
clean:
	-rm *.o
	-rm *.obj

cleaner:
	-rm *.*~
	-rm *.flc


distribution:


	
