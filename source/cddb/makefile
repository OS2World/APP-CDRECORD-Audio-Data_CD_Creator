# Makefile for Audio-CD copy

CC           = icc
LIB_DIR      = $d:/emx/lib 
CPPFLAGS     = -Ge- /Gm+ -G4 -Gl  /O+ -Oc+ -Ol
INC		= ../include
DEFS         = 
PROGS        =  
ODIR		= .
CPPDIR	= .
BINDIR		=	../../dist/bin

CPPFILES	=
OBJECTS	=	 cddb.obj printHelper.obj

all:	 querycddb.exe cddbcls.obj

querycddb.exe:	cddb.obj $(OBJECTS) $(INC)\cddb.hh querycddb.cpp
		gcc querycddb.cpp ..\helper\cd.cpp $(OBJECTS) -I$(INC) -Zcrtdll -Zomf -lsocket -Zlinker /PMTYPE:VIO

# cddb.def

cddb.obj:	cddb.cpp $(INC)\cddb.hh
		gcc -c cddb.cpp -I$(INC) -Zcrtdll -Zomf

cddbcls.obj:	cddbcls.cpp $(INC)\cddb.hh $(INC)\cddb.h
		cd ../pm_launchers && make

printHelper.obj:	printHelper.cpp
		gcc -c printHelper.cpp -I$(INC) -Zcrtdll -Zomf

clean:
	-rm *.obj
	-rm *.exe

cleaner:
	-rm *.*~

distribution:
	-cp -f querycddb.exe $(BINDIR)
	
