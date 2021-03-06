� �� 0#   d    k   j   g   i   h   f   ��� 0�  init�/* Load Rexxutil functions */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs


cdrList.1="cdrecord 1.8a31b"
cdrList.1._supported='YES'
cdrList.1._graftPoints='CLEAR'

cdrList.2="cdrecord 1.8a32"
cdrList.2._supported='YES'
cdrList.2._graftPoints='CLEAR'

cdrList.3="cdrecord 1.8a40"
cdrList.3._supported='YES'
cdrList.3._graftPoints='CLEAR'

cdrList.4="cdrecord 1.8.1a05"
cdrList.4._supported='YES'
cdrList.4._graftPoints='CLEAR'

cdrList.5="cdrecord 1.9a02"
cdrList.5._supported='YES'
cdrList.5._graftPoints='CLEAR'

cdrList.6="cdrecord 1.10a15"
cdrList.6._supported='YES'
cdrList.6._graftPoints='-graft-points'
cdrList.7="cdrecord 1.11a6"
cdrList.7._supported='YES'
cdrList.7._graftPoints='-graft-points'

cdrList.8="cdrecord 2.00 final"
cdrList.8._supported='YES'
cdrList.8._graftPoints='-graft-points'


/* Not supported: */
cdrList.9="cdrtools2 1.10"
cdrList.9._supported='NO'
cdrList.9._graftPoints=' '

cdrList.10="cdrtools2 1.11a04"
cdrList.10._supported='NO'
cdrList.10._graftPoints=' '

cdrList.11="cdrtools2 1.11a07"
cdrList.11._supported='NO'
cdrList.11._graftPoints=' '

cdrList.12="cdrtools2 1.11a13"
cdrList.12._supported='NO'
cdrList.12._graftPoints=' '

cdrList.13="cdrtools2 1.11a16"
cdrList.13._supported='NO'
cdrList.13._graftPoints=' '

cdrList.14="Any other cdrtools2 release" 
cdrList.14._supported='NO'
cdrList.14._graftPoints=' '


cdrList.0=14
�� �d 0]  ]  �  ��          �   �| 8 � � d ���         � @� � � 4 k ����      %   �  � 2 � � j  ��          �  >  g ����        (  ��  >  i ����        8  �  .  h ����        >  �  � � f ����Creator configuration    Text           ����      ����Set options More options... Close Select your CDRecord/2 release ���d 0�  �h ClickExit
Initcall text '~Close'�i Click>call SysSetObjectData '<CWCREATOR_SETTINGS>', 'OPEN=DEFAULT'
Initcall text 'More options...'�g Click�sel=d100.c106.Select()

IF sel\=0 THEN DO
   call d100.c106.GetStem data , sel
   i=data.3
   IF TRANSLATE(data.2)='NO' THEN DO
	rc=RxMessageBox('This release of cdrecord is not supported. It may work or may not work. Use it on your own risk.',"Information", 'OK', 'EXCLAMATION')
   END
   setup='MKISOFSOPTIONS='||cdrlist.i._graftPoints
   ret=SysSetObjectData('<CWCREATOR_SETTINGS>', setup)
END
Initcall TEXT '~Set options'�j Init�/* Set format */
call view 'D'
data.0=3
data.1='x_!'
data.2='x_!'
data.3='~'

call SetStem data, 'format'

/* Set title */
data.0=3
data.1='Release'
data.2='Supported'
data.3=''
call SetStem data, 0


/* Set items */
DO i=1 to cdrList.0
	item=Add(i)
	data.0=3
	data.1=cdrList.i
	data.2=cdrList.i._supported
	data.3=i
	call SetStem data, item
END
�k Init�call Text 'Select your CDRecord/2 release and press <Set options> to configure some essential options. You HAVE to set necessary paths and additional options in the settings notebook. Open it by pressing <More options...>.'�