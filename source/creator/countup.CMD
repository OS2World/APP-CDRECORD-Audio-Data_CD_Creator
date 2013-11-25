/* rexx -  count up the build */

/* Build path to counterfile */
curdir=directory()
lastBackSlash=LASTPOS('\',curdir)
counterfile=LEFT(curdir,lastBackSlash) ||'include\buildcount.h'


/* Check if we already have a counter */
rc=CHARS(counterfile)
if rc=0 then
do
   rc=CHAROUT(counterfile, '1' )
   exit
end

/* counter=counter +1 */
rc=CHARIN(counterfile, '1', rc )
rc=CHAROUT(counterfile, rc+1,'1' )
/* rc=CHARS(counterfile)
rc=CHARIN(counterfile, '1', rc )
say rc */

