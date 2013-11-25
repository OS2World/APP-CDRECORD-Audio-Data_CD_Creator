################################################
#
#                       Makefile for class CWProgFolder 
#
#                          (C) Chris Wohlgenuth 1999-2001
# 
#          E-mail:    christopher.wohlgemuth@bch.siemens.de
#
#          http://www.geocities.com/SiliconValley/Sector/5785/
#
#
# This class is intended for programmers and provides functions for some
# common tasks like:
#                     - Adding dialogs as frame controls to WPS folders
#                     - Showing 'About'-dialogs
#                     - Checking of object pointers
#                     - ...
#
# The DLL also includes some other functions like:
#
#                     - Query CD drives
#                     - Query audio CD tracks
#                     - Query audio CD track size
#                     - ...
#
# Use IBM VAC++ V3.00 and Gnu-make 3.74.1 to compile
#
#
#
#
################################################

#/*
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2, or (at your option)
# * any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; see the file COPYING.  If not, write to
# * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
# */

include ../make.inc


CC           = icc
LIB_DIR      = $d:/emx/lib 
CPPFLAGS     = /Ge- /Gm+ -G4 -Gl /O+ -Oc+ /C+ /Q+
INC		= ../include
INC2		= ..\include
DEFS         = 
PROGS        =  
ODIR		= .
CPPDIR	= .
OBJECTS	= progfolder.obj pframeproc.obj storewindow.obj separatorwindow.obj
DISTDIR	=	../../dist
DLLDIR		=	../../dist/DLL

all:	 progfolder.dll

progfolder.dll:	$(OBJECTS) progfolder.def
		icc -Ge- /Q+ $(OBJECTS) progfolder.def
		@echo "[33;1;m  Adding resources...[0;m"
		@rc -i $(INC2) progfolder.rc progfolder.dll >NUL
		@implib /NOLOGO progfldr.lib progfolder.def
		
progfolder.obj:		progfolder.cpp $(INC)/progfolder.hh $(INC)/progfolder.h
	icc $(CPPFLAGS) -I$(INC) $<

pframeproc.obj:		pframeproc.cpp $(INC)/progfolder.hh $(INC)/progfolder.h
	icc $(CPPFLAGS) -I$(INC) $< 

storewindow.obj:	storewindow.cpp $(INC)/progfolder.hh $(INC)/progfolder.h
	icc $(CPPFLAGS) -I$(INC) $< 

separatorwindow.obj: separatorwindow.cpp $(INC)/progfolder.hh $(INC)/progfolder.h
	icc $(CPPFLAGS) -I$(INC) $< 

clean:
	-rm *.obj
	-rm *.dll
	-rm *.lib

cleaner:
	-rm *.*~
	-rm *.flc

distribution:
	-cp progfolder.dll $(DLLDIR)/progfldr.dll

