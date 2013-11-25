/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

conffile="Cfgdata.ini"
outfile="drivedata_.txt"

if SysIni(conffile, "ALL:", "apps.")\="ERROR:" THEN DO
DO a=1 to apps.0
	/* Found all apps */
	SAY "Processing "apps.a"..."
	if SysIni(conffile, apps.a, "ALL:", "keys")\="ERROR:" THEN DO
		call LINEOUT outfile, '['||apps.a||"]"
		DO b=1 to keys.0
			SAY "  "keys.b" "SysIni(conffile, apps.a, keys.b)
			call LINEOUT outfile, keys.b||" "||SysIni(conffile, apps.a, keys.b)
		END
	END
	call LINEOUT outfile, ""
END
END

call stream outfile, 'C', 'close'
