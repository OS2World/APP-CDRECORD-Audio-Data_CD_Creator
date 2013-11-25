/* */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

SAY 'Checking for registration...'
SAY 'DRCtrlLoadFuncs:'
SAY RXFUNCQUERY('DRCtrlLoadFuncs')
SAY RXFUNCDROP('DRCtrlLoadFuncs')
SAY RXFUNCQUERY('DRCtrlLoadFuncs')
SAY 'DRCtrlVersion:'
SAY RXFUNCQUERY('DRCtrlVersion')
SAY RXFUNCDROP('DRCtrlVersion')
SAY RXFUNCQUERY('DRCtrlVersion')
SAY 'DRCtrlDropFuncs:'
SAY RXFUNCQUERY('DRCtrlDropFuncs')
SAY RXFUNCDROP('DRCtrlDropFuncs')
SAY RXFUNCQUERY('DRCtrlDropFuncs')

SAY 'Register...'
call RxFuncAdd 'DRCtrlLoadFuncs', 'drusrctl', 'DRCtrlLoadFuncs'
rc=DRCtrlLoadFuncs()

call SysSleep 1

SAY 'Checking for registration...'
SAY 'DRCtrlLoadFuncs: 'RXFUNCQUERY('DRCtrlLoadFuncs')
SAY 'DRCtrlVersion: 'RXFUNCQUERY('DRCtrlVersion')
SAY 'DRCtrlDropFuncs: 'RXFUNCQUERY('DRCtrlDropFuncs')

call DRCtrlDropFuncs
SAY 'DeRegistered'

SAY 'DRCtrlLoadFuncs: 'RXFUNCQUERY('DRCtrlLoadFuncs')
SAY RXFUNCDROP('DRCtrlLoadFuncs')
SAY RXFUNCQUERY('DRCtrlLoadFuncs')
SAY 'DRCtrlVersion: ' RXFUNCQUERY('DRCtrlVersion')
SAY RXFUNCDROP('DRCtrlVersion')
SAY RXFUNCQUERY('DRCtrlVersion')
SAY 'DRCtrlDropFuncs: 'RXFUNCQUERY('DRCtrlDropFuncs')
SAY RXFUNCDROP('DRCtrlDropFuncs')
SAY RXFUNCQUERY('DRCtrlDropFuncs')
