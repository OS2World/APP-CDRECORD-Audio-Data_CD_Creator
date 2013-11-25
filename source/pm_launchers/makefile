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
RESDIR	=	..\res\$(LANGUAGE)
ICONDIR =	..\res\Icons
HELPERDIR	=	..\launch_helper
CDDBDIR	=	..\cddb
OBJECTS	=	$(HELPERDIR)\message.obj $(HELPERDIR)\helper.obj 
CDDBOBJ	=	cddb.obj

all:	pmthefly.exe pacdgrab.exe pwrteimg.exe pmwrtaud.exe pmcddb.exe \
	ptrkname.exe pmmp3dec.exe pmdaytip.exe\
	pmwvinfo.exe pacdgrb2.exe pmmntiso.exe pmunmnt.exe pmhint.exe pmdvdfly.exe
# pmtst.exe
#pmaudcpy.exe

pmtst.exe:	pmtst.c
	gcc -O -Zomf -Zcrtdll -s $<  pmtst.def
	rc pmtst.res pmtst.exe 

pmmp3dec.exe:	pmmp3dec.c $(OBJECTS) $(HELPERDIR)\progbars.obj
	gcc $(CPPFLAGS) -I$(INC) pmmp3dec.c $(OBJECTS) $(HELPERDIR)\progbars.obj pmmp3dec.def

pmwvinfo.exe:	pmwvinfo.c $(OBJECTS)
	gcc $(CPPFLAGS) -I$(INC) pmwvinfo.c $(OBJECTS) -los2me pmwvinfo.def

#pmaudcpy.exe:	pmaudcpy.c $(OBJECTS) 
#	gcc $(CPPFLAGS) -I$(INC) pmaudcpy.c $(OBJECTS) pmaudcpy.def

ptrkname.exe:	ptrkname.c $(OBJECTS) 
	gcc $(CPPFLAGS) -I$(INC) ptrkname.c $(OBJECTS) ptrkname.def

pacdgrab.exe:	pacdgrab.c $(OBJECTS) 
	gcc $(CPPFLAGS) -I$(INC) pacdgrab.c $(OBJECTS) pacdgrab.def

pacdgrb2.exe:	pacdgrb2.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj
	gcc $(CPPFLAGS) -I$(INC) pacdgrb2.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj pacdgrb2.def

pmthefly.exe:	pmthefly.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj
	gcc $(CPPFLAGS) -I$(INC) pmthefly.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj pmthefly.def

pmdvdfly.exe:	pmdvdfly.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj
	gcc $(CPPFLAGS) -I$(INC) pmdvdfly.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj pmdvdfly.def

pwrteimg.exe:	pwrteimg.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj
	gcc $(CPPFLAGS) -I$(INC) pwrteimg.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj pwrteimg.def

pmwrtaud.exe:	pmwrtaud.c $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj
	gcc $(CPPFLAGS) -I$(INC) $<  $(OBJECTS) $(HELPERDIR)\progbars.obj $(HELPERDIR)\pmhelper_customprocs.obj pmwrtaud.def

pmcddb.exe:	pmcddb.cc $(OBJECTS) $(CDDBOBJ) $(INC)\cddb.h $(INC)\cddb.hh cd.obj $(HELPERDIR)\cddbhelper.obj
	gcc $< $(CPPFLAGS) -I$(INC)   $(OBJECTS) $(CDDBOBJ) $(HELPERDIR)\cddbhelper.obj cd.obj -lsocket pmcddb.def

pmmntiso.exe:	pmmntiso.c $(OBJECTS) pmmntiso.def
	gcc $< $(CPPFLAGS) -I$(INC) $(OBJECTS) pmmntiso.def

pmunmnt.exe:	pmunmnt.c $(OBJECTS) pmunmnt.def
	gcc $< $(CPPFLAGS) -I$(INC) $(OBJECTS) pmunmnt.def

pmhint.exe:	pmhint.c pmhint.def $(OBJECTS) $(RESDIR)/pmhint.dlg pmhint.rc pmhint.def
	gcc $< $(CPPFLAGS) -I$(INC) $(OBJECTS) pmhint.def
	-cp ../res/hand.ptr hand.ptr
	-cp $(RESDIR)/pmhint.dlg pmhint.dlg
	rc pmhint.rc pmhint.exe

pmdaytip.exe:	pmdaytip.c pmdaytip.def $(OBJECTS) $(RESDIR)/pmdaytip.dlg pmdaytip.rc pmdaytip.def
	gcc $< $(CPPFLAGS) -I$(INC) $(OBJECTS) pmdaytip.def
	-cp ../res/hand.ptr hand.ptr
	-cp $(RESDIR)/pmdaytip.dlg pmdaytip.dlg
	-cp $(ICONDIR)/info.ico info.ico
	rc pmdaytip.rc pmdaytip.exe

$(HELPERDIR)\message.obj:	$(HELPERDIR)\message.c 
	gcc -c -o$@ -Zomf -I$(INC) $< 		

$(HELPERDIR)\cddbhelper.obj:	$(HELPERDIR)\cddbhelper.cpp 
	gcc -c -o$@ $(CPPFLAGS) -I$(INC) $< 		

$(HELPERDIR)\helper.obj:	$(HELPERDIR)\helper.c
	gcc -c -o$@ -Zomf -I$(INC) $< 		
	
$(HELPERDIR)\progbars.obj:	$(HELPERDIR)\progbars.c
	gcc -c -o$@ -Zomf -I$(INC) $< 		

$(HELPERDIR)\pmhelper_customprocs.obj:	$(HELPERDIR)\pmhelper_customprocs.c
	gcc -c -o$@ -Zomf -I$(INC) $< 		

$(CDDBOBJ):	..\cddb\cddb.cpp
	gcc -c -o$@ $(CPPFLAGS) -I$(INC) $< 		

cd.obj:	..\helper\cd.cpp
	gcc -c -o$@ $(CPPFLAGS) -I$(INC) $< 		

message.obj:	..\helper\messagehelper.cpp 
	gcc -c -o$@ -Zomf -I$(INC) $< 		

helper.obj:	..\helper\helper.cpp
	gcc -c -o$@ -Zomf -I$(INC) $< 		

clean:
	-rm *.exe
	-rm *.o
	-rm *.obj
	-rm $(HELPERDIR)/*.o
	-rm hand.ptr
	-rm *.ico

cleaner:
	-rm *.*~
	-rm *.flc
	-rm $(HELPERDIR)\*.*~
	-rm $(HELPERDIR)\*.flc

distribution:
	-cp *.exe ../../dist/bin
	
