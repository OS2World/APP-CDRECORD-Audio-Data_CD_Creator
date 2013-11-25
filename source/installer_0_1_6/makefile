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
DISTDIR2	=	..\..\dist
INC		=	..\include
RES		=	..\res


all:	
#install.exe

#$(INC)/.h

install.exe:	install.c $(INC)/installer.h $(RES)\$(LANGUAGE)\installer$(LANGUAGE).rc
		icc -Ol -I$(INC) install.c installer.def
		-cp ../res/Icons/audiofolder1.ico ./
		-cp ../res/Icons/datafolder1.ico ./
		rc -i $(INC) $(RES)\$(LANGUAGE)\installer$(LANGUAGE).rc install.exe
#		rc -DINCL_ICON installer.res install.exe
#		-cp $(RES)/cdrtools.ico ./

clean:
	-rm *.obj
	-rm *.ico
#	-rm install.exe

cleaner:
	-rm *.*~
	-rm *.flc

distribution:
#	-cp ./install.exe $(DISTDIR)/inst/pmconfig.exe
#	-cp install.cmd $(DISTDIR)/
	-cp adc.in_ $(DISTDIR)/adc.in_
	-cp adc2.in_ $(DISTDIR)/adc2.in_
	-cp install.in_ $(DISTDIR)/install.in_
	-cp ./install.exe $(DISTDIR2)/install.exe
	-cp uninstal.cmd $(DISTDIR)/inst
	-cp cfgdata.ini $(DISTDIR)/
	-cp -R install $(DISTDIR)/
	-cp Howto-Install.cmd $(DISTDIR)
	-cp ./install/cwinst2.in_ $(DISTDIR)/cwinst2.in_
