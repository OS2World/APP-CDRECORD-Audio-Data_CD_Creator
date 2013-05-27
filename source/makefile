# Makefile for Audio-CD-Creator/Data-CD-Creator

include ./make.inc

DIST_NAME	= audiocdcreator_$(CDCVERSION).zip
SRC_NAME	= audiocdcreator_$(CDCVERSION)_src.zip
INC		=	.\include

all:	progfldr.dll aucdfldr.dll launchprogs installProg cddbqry hint cfgdata
#(cdrtools is obsolete today, the functionality is included into the classes)

aucdfldr.dll:	
		@echo "[35;1;mCompiling Audio-Creator DLL.[0;m"
		@cd creator && make
#		cd helper && make

progfldr.dll:
		@echo "[35;1;mCompiling CWProgFolder.[0;m"
		@cd Progfolder && make

launchprogs:
		@echo "[35;1;mCompiling VIO launchers.[0;m"
		@cd vio_launchers && make
		@echo "[35;1;mCompiling PM launchers.[0;m"
		@cd pm_launchers && make

installProg:
		cd installer && make

cddbqry:
		@echo "[35;1;mCompiling CDDB query program.[0;m"
		@cd cddb && make

hint:
#		@cd res && make


cfgdata:
		cd drive_data && make

clean:
	-cd CDRTOOLS && make clean
	-cd Progfolder  && make clean
	-cd creator && make clean
	-cd audiocreator && make clean
	-cd creatorsettings && make clean
	-cd pm_launchers && make clean
	-cd vio_launchers && make clean
	-cd helper && make clean
	-cd launch_helper && make clean
	-cd installer && make clean
	-cd cddb && make clean

cleaner:
	cd CDRTOOLS && make cleaner
	cd Progfolder  && make cleaner
	cd creator && make cleaner
	cd audiocreator && make cleaner
	cd datacreator && make cleaner
	cd pm_launchers && make cleaner
	cd vio_launchers && make cleaner
	cd helper && make cleaner
	cd launch_helper && make cleaner
	cd installer && make cleaner
	cd cddb && make cleaner
	-rm -r ../dist/*


distribution:
	make all
	-rm -r ../dist/*
	-mkdir ../dist/bin
#	-mkdir ../dist/bin/toolbar
	-mkdir ..\dist\inst
	-mkdir ..\dist\DLL
	-mkdir ..\dist\Temp
	-mkdir ..\dist\Help
	-mkdir ..\dist\scripts
	-cd progfolder && make distribution		
	-cd creator && make distribution
	-cd pm_launchers && make distribution
	-cd vio_launchers && make distribution
	-cd installer && make distribution
	-cd cddb && make distribution
	-mkdir ..\dist\Docs
	-mkdir ..\dist\Logfiles
	-mkdir ..\dist\Res
	-mkdir ..\dist\cddbdata
	-cp -f drive_Data/cfgdata.ini ../dist
	-cp -f ../doc/creator/* ../dist/docs
	-cp -f -R ../doc/mpg123 ../dist/docs
	-cp -f ../doc/cdwriting.htm ../dist/Help
	-cp -f ../doc/Readme.1st ../dist
	-cp -f  scripts/* ../dist/scripts
	-rm ../dist/scripts/adcscr.in_
# Add mp3info to distribution
	-cp -f -R mp3info ../dist/docs
	-rm -r ../dist/docs/mp3info/source
	-rm  ../dist/docs/mp3info/mp3info.exe
	-rm  ../dist/docs/mp3info/mp3pls.cmd
	-cp -f mp3info/mp3info.exe ../dist/bin
	-cp -f mp3info/mp3pls.cmd ../dist/bin
	-cp -f -R  ./precompiled/* ../dist/bin
	-rm -f ../dist/bin/*.dlg
	-rm ../dist/bin/creat*.cmd
	-cp helper/drusrctl.dll ../dist/bin
	-cp -f  precompiled/creat*.cmd ../dist/inst
# Add the icons to the distribution
	-cp -f res/icons/cdrfold1.ico  ../dist/Res
	-cp -f res/icons/cdrfold2.ico  ../dist/Res
	-cp -f res/icons/tutorial.ico  ../dist/Res
	-cp -f res/icons/cdTOOLS1.ico  ../dist/Res/cdTOOLS1.ico
	-cp -f res/icons/cdtools2.ico  ../dist/Res/cdTOOLS2.ico
	-cp -f res/icons/pmaudcpy.ico  ../dist/Res
	-cp -f res/icons/cdrtools.ico  ../dist/Res
	-cp -f res/icons/mp3.ico  ../dist/Res
	-cp -f res/icons/info.ico  ../dist/Res
	-cp -f res/icons/help.ico  ../dist/Res
	-cp -f res/icons/cover.ico  ../dist/Res
	-cp -f res/icons/cd.ico  ../dist/Res/cdprops.ico
	-cp -f res/icons/*.jpg  ../dist/Res
	-cd ..\dist\DLL && lxlite *.dll
	-cd ..\dist && lxlite *.dll
	-cd ..\dist && lxlite *.exe
	-cd ..\dist\bin && lxlite *.exe
# Add batchburn
	-cp -f  batchburn-2_0.zip ../dist
# Zip it
	-cd ..\dist && zip -r $(DIST_NAME) *
	

sourcedistribution:
	make clean
	make cleaner
	-rm include/*.*~
	-rm include/*.flc
	cd ../. && zip -r $(SRC_NAME) *

help:
	@echo "Audio/Data-CD-Creator V"$(CDCVERSION)
	@echo ""
	@echo "'make'                 Compile only"
	@echo "'make help'            This screen"
	@echo ""
	@echo "Other targets:"
	@echo "'distribution'         Create the ADC-distribution"
	@echo "'sourcedistribution'   Create the sourcecode distribution"
	@echo "'installProg'          Compile the PMinstaller"
	@echo "'clean'"
	@echo "'cleaner'"
