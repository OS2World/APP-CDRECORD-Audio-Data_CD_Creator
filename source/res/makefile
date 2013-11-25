# Makefile for Audio-CD-Creator Data-CD-Creator

include ./../make.inc

CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol- /Q+ $(VACDEBUG)
                  
INC	=	../include
RESDIR	=	./$(LANGUAGE)
OBJDIR		=	./obj
OBJ	=

all:	$(OBJDIR)/audiofolder.res \
	all_lang \
	$(LANGUAGE)/daytip.ini \
	$(LANGUAGE)/ipf/tutorial$(LANGUAGE).inf

all_lang:
	@echo "[35;1;mCreating laguage files.[0;m"
	@for i in $(LANGUAGES); \
	do \
		echo "[33;1;m Creating audiofolder"$$i".res[0;m";  \
		make LANGUAGE=$$i $(OBJDIR)/audiofolder$$i.res > NUL; \
	done
	@for i in $(LANGUAGES); \
	do \
		echo "[33;1;m Creating cdfld"$$i".dll[0;m" \
		& make $(OBJDIR)/cdfld$$i.dll >NUL; \
	done
	@for i in $(LANGUAGES); \
	do \
		echo "[33;1;m Creating hint_"$$i".dll[0;m"; \
		make $(OBJDIR)/hint_$$i.ini >NUL; \
	done


# Base resources to be bound to class dll
$(OBJDIR)/audiofolder.res:	./base/audiofolder.rc
		@cd base && rc -i ../$(INC) -r audiofolder.rc  ../$(OBJDIR)/audiofolder.res >NUL

#NLV resources bound to a resource dll
$(OBJDIR)/audiofolder$(LANGUAGE).res:		$(LANGUAGE)/audiofolder$(LANGUAGE).rc $(LANGUAGE)/audiofolder$(LANGUAGE).dlg $(INC)\audiofolderres.h
		cd $(LANGUAGE) && rc -i ../$(INC) -r audiofolder$(LANGUAGE).rc ../$(OBJDIR)/audiofolder$(LANGUAGE).res

# The Language specific dll
$(OBJDIR)/cdfld$(LANGUAGE).dll:	./$(LANGUAGE)/cdfld$(LANGUAGE).c \
					$(LANGUAGE)\cdfld$(LANGUAGE).def \
					$(OBJDIR)\audiofolder$(LANGUAGE).res
		icc $(CPPFLAGS) -I$(INC) -c /Fo$(OBJDIR)/cdfld$(LANGUAGE) $< 
		icc $(CPPFLAGS) -I$(INC) /Fe$@ $(OBJDIR)/cdfld$(LANGUAGE).obj $(LANGUAGE)\cdfld$(LANGUAGE).def
		rc $(OBJDIR)\audiofolder$(LANGUAGE).res $(OBJDIR)\cdfld$(LANGUAGE).dll

# Hints
$(OBJDIR)/hint_$(LANGUAGE).ini:	$(LANGUAGE)/hintdata.txt $(LANGUAGE)/nls_$(LANGUAGE).txt
		CMD.EXE /C buildhint.cmd $(LANGUAGE)\hintdata.txt, $(OBJDIR)\HINT_$(LANGUAGE).ini
		cmd.exe /C addnlsstrings $(LANGUAGE)\nls_$(LANGUAGE).txt , $(OBJDIR)\HINT_$(LANGUAGE).ini

#Daytip
$(LANGUAGE)/daytip.ini:	$(LANGUAGE)/daytipdata.txt
		@echo "[33;1;m Creating "$@"[0;m"
		@cd $(LANGUAGE) && cmd.exe /C builddaytip.cmd >NUL

#Tutorial and help
$(LANGUAGE)/ipf/tutorial$(LANGUAGE).inf:	$(LANGUAGE)/ipf/tutorial$(LANGUAGE).ipf
	@echo "[33;1;m Creating "$@"[0;m"
	@cd ./$(LANGUAGE)/ipf && make >NUL


clean:
	-cd $(LANGUAGE) && rm *.ini
	-cd $(LANGUAGE)/ipf && rm *.hlp
	-cd $(LANGUAGE)/ipf && rm *.inf
	-cd $(OBJDIR) && rm *.*

cleaner:
	-cd $(LANGUAGE)/ipf && rm *.*~
	-cd $(LANGUAGE) && rm *.flc
	-cd $(LANGUAGE) && rm *.*~
	-cd base && rm *.*~


distribution:
	-cp $(LANGUAGE)/*.ini ./../../dist/Help/
	-cp $(OBJDIR)/hint_$(LANGUAGE).ini ./../../dist/Help/hint.ini
	
