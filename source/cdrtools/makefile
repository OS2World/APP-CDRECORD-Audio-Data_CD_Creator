#                          Makefile for CDRTools
#
#                     (C) Chris Wohlgenuth 1998-99
#
#          E-mail:    christopher.wohlgemuth@bch.siemens.de
#
#          http://www.geocities.com/SiliconValley/Sector/5785/
#

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

CC		=	icc
DISTDIR	=	../../dist
INC		=	..\include
RES		=	..\res


all:	cdrtools.exe

cdrtools.exe:	cdrtools.cpp $(INC)/audiodialog.h 
		icc -G4 -Gl -Rn /O+ -Oc+ -Ol /Q+ -I$(INC) cdrtools.cpp cdrtools.def
		-cp $(RES)/Icons/cdrtools.ico ./
		rc -i $(INC) $(RES)\$(LANGUAGE)\cdrtoolsdialog$(LANGUAGE).rc cdrtools.exe

clean:
	-rm *.obj
	-rm *.ico
	-rm *.exe
	-rm $(RES)/$(LANGUAGE)/*.res

cleaner:
	-rm *.*~
	-rm *.flc

distribution:
	-cp *.exe $(DISTDIR)/CDRTools.exe

