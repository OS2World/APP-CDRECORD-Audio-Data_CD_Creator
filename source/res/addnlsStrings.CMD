/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

ARG datafile ',' hintini 

datafile=DIRECTORY()||'\'||datafile
hintini=DIRECTORY()||'\'||hintini

/*
IF ARG()=1 THEN DO
	datafile=ARG(1)
END
ELSE DO
	datafile=nls001.txt
END
	hintini='hint.ini'
*/

index=0

SAY 'Datafile with NLS strings: '||datafile
SAY ''
SAY ARG()
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
RETURN


parseapp:
 app=STRIP(TRANSLATE(data.currentline,'  ','[]'))
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
              otherwise
		section=replaceCR(section)	
		say app||": "||section
		SAY ""
/*		section=section||'00'X*/
		rc=SysIni(hintini, "nls", word(section,1), STRIP(subword(section, 2)))
                  /* Parse source and target */
               end
           currentline=currentline+1
        end
return


replaceCR:

	string=STRIP(ARG(1))

	thePos=WORDPOS("0ax",string)
	if thePos\=0 THEN DO
		string=SUBWORD(string,1,thePos-1)||'0a'x||replaceCR(SUBWORD(string,thePos+1))
	END

return string

