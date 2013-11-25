/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

datafile=daytipdata.txt
hintini=daytip.ini
index=0

SAY 'Datafile with daytips: '||datafile

/* Read the datafile */
    i = 0
    do while lines(datafile) <> 0
       i=i+1
       data.i = linein(datafile)
    END
    data.0=i

   call stream datafile, 'C', 'CLOSE'

    /* Correct tip numbers */
    currentline=1
    do while currentline <= data.0
      section = Strip(Translate(word(data.currentline,1)))
	select
	    when (Left(section,1) = '#') | section = '' then
		do
		/* Jump comments and empty lines */
		end;
              otherwise
		IF POS('[',section)=1 THEN DO
			index=index+1
			data.currentline='['index']'
		END
           end
           currentline=currentline+1
        end

    currentline=1
    do while currentline <= data.0
      section = Strip(Translate(word(data.currentline,1)))
	select
	    when (Left(section,1) = '#') | section = '' then
		do
		/* Jump comments and empty lines */
		end;

              otherwise
		call parseapp currentline
		   nop
           end
           currentline=currentline+1
        end

/* Set general info like number of tips */
	rc=SysIni(hintini, "daytip", "numtips", index)
/*	rc=SysIni(hintini, "daytip", "currenttip", index)
*/


RETURN

parseapp:
 app=STRIP(TRANSLATE(data.currentline,'  ','[]'))
 say app
 currentline=currentline+1
    do a=currentline to data.0
      section = Strip(data.a)
	select
	    when (Left(section,1) = '#') | section = '' then
		do
			/* Jump comments and empty lines */
		end;
                WHEN (Left(section,1) = '[') THEN DO
                    /* Next section reached */
                        currentline=currentline-1
			return
                  END
		WHEN Left(section,4) = "FILE" THEN DO

                END
              otherwise
		crPos=WORDPOS("0Dx", section)
		/* Build CRs */
		if crPos\=0 THEN DO
		DO WHILE crPos\=0
			section=SUBWORD(section,1,crPos-1)||'0D'x||' '||SUBWORD(section,crPos+1)
			if WORDPOS("0Dx",SUBWORD(section,crPos+1))\=0 THEN
				section=SUBWORD(section,1,crPos-1)||'0D'x||' '||SUBWORD(section,crPos+1)
			else
				section=SUBWORD(section,1,crPos-1)||'0D'x||SUBWORD(section,crPos+1)
			crPos=WORDPOS("0Dx", section)
		END
		END
		say section
		section=section||'00'X
		rc=SysIni(hintini, app, word(section,1), STRIP(subword(section, 2)))
                  /* Parse source and target */
               end
           currentline=currentline+1
        end
return

