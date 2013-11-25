/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

datafile=drivedata.txt
if ARG()\=1 THEN
	hintini="test.ini"
ELSE
	hintini=ARG(1)

SAY 'Datafile with config info: '||datafile
SAY 'Ini file name: 'hintini
SAY ''

i = 0
do while lines(datafile) <> 0
       i=i+1
       data.i = linein(datafile)
END
data.0=i

call stream datafile, 'C', 'CLOSE'


    currentline=1
    do while currentline <= data.0
      section = Strip(Translate(word(data.currentline,1)))
	select
	    when (Left(section,1) = '#') | section = '' then
		do
		/* Jump comments and empty lines */
		end;
/*
                WHEN section='[APPLICATION]' THEN DO
                  call readMainAppInfo
                END
*/
              otherwise
			call parseapp currentline
		   nop
           end
           currentline=currentline+1
        end

RETURN

parseapp:
	/* Remove [] */
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
                    /* Next section reached. */
                        currentline=currentline-1
			return
                  END
              otherwise
		say section
		section=section||'00'X
		if STRIP(subword(section, 2))\='' THEN
			rc=SysIni(hintini, app, word(section,1), STRIP(subword(section, 2)))
		ELSE
			/* Handle empty cdrdaooptions */
			rc=SysIni(hintini, app, word(section,1), " ")
               end
           currentline=currentline+1
        end
return

