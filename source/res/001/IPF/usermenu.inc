:h2 res=853
x=right y=bottom
width=65%
height=100%
group=2.User menu
:i1 .User menu
:i2 refid=customization.User menu
:p.
If you've installed WPS-Wizard you may configure a user menu for the
creator classes. 
:p.
:artwork  name='usermenu.bmp'.
:p.
Get this WPS extension from the following locations.
:p.
:link reftype=launch
object='netscape.exe'
data='http://www.geocities.com/SiliconValley/Sector/5785/index.html'.
http&colon.//www.geocities.com/SiliconValley/Sector/5785/index.html:elink.
.br
:link reftype=launch
object='netscape.exe'
data='http://www.os2world.com/cdwriting'.
http&colon.//www.os2world.com/cdwriting/:elink.


:nt.If WPS-Wizard isn't installed when installing Audio/Data-Cd-Creator
an object is created to add the necessary configuration folders later.:ent.

:p.
Open the :hp2.Configuration and (Un)Installation:ehp2. folder in the
:hp2.CD-Writing:ehp2. folder to find the configuration folder for the
user menu. Customization of the user menu is mainly done as described
in the tutorial coming with WPS-Wizard. There's one difference for
audio-folders though.

:p.
When selecting the menu entry of an audio-folder menu a special file
is given as the parameter to the program objects of the menu. Normaly
the name and path of the folder is the parameter and in fact that's
the way the menu works for data-folders. This special file is a text
file which may be parsed by a script to get information about the
contents of a folder. After doing so the script may for example
convert all wave files to MP3-files or lower the amplitude of a
wave.
:nt.The parameter file will be deleted by the class after 2 minutes. So be
sure to save the contents for later use in your script.:ent.


