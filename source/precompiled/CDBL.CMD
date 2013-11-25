/* rexx */
/* load a cdboot.img file to a Virtual Disk */
vfd14dl = 'P:'  /* This is the 1.44MB Virtual Disk */
vfd28dl = 'O:'  /* This is the 2.88MB Virtual Disk */
defaultfilename = 'cdboot.img'

/* init */
filename = ''
driveletter = ''

/* get cmdline parameters */
parse arg filename driveletter .
if filename = '' then filename = defaultfilename
if driveletter = '' then
do
  /* get file size */
  rc = SysFileTree(filename,'file','F')
  if file.0 = 0 then 
  do
    say 'Error: File "'filename'" not found in 'directory()'!'
    exit(1)
  end
/*  say file.1 */
  parse value file.1 with date time size attribs fn
  if strip(size) = '2949120' then driveletter = vfd28dl
  if strip(size) = '1474560' then driveletter = vfd14dl
  if driveletter = '' then
  do
    say 'Error: A disk image must be either 1474560 or 2949120 bytes in size.'
    Say directory()'\'File' is 'strip(size)' Bytes.'
    exit(2)
  end
end
'loaddskf.exe 'filename' 'driveletter' /y /q'

exit(0)
