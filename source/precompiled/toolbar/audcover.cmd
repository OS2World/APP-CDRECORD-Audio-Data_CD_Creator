/* Create an audio cover */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

IF ARG()=0 THEN EXIT

ret=SysSetObjectData(WPSWizCallFunc("cwQueryRealName", ARG(2), 1), "MENUITEMSELECTED=6613")

exit
