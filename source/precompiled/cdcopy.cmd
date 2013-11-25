/* This REXX skript handles the cd copying with cdrdao   */

/*
 * This file is (C) Chris Wohlgemuth 2000-2001
 *
 * It's part of the Audio/Data-CD-Creator distribution.
 *
 * Visit
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/index.html
 *
 * for more information.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

/* Parse the parameters */
parse arg targetDevice imageDrive' "'installDir'" 'simulate

inifile=installDir||'\CDRECORD.INI'
cdrdaoOptions=SysIni(inifile, 'cdrdao', 'options')
if cdrdaoOptions='ERROR:' THEN cdrdaoOptions=''
else
cdrdaoOptions=LEFT(cdrdaoOptions, LENGTH(cdrdaoOptions)-1)


SAY 'Skript for CD copy started'
SAY ''
SAY 'Target: 'targetDevice                       /* The device to write on             */
SAY 'Drive for Image: 'imageDrive                /* The drive where to write the image */
SAY 'Installation directory: 'installDir         /* Installation directory             */
SAY 'Simulate='simulate                          /* 1: simulate, 0: write              */
SAY 'Options: 'cdrdaoOptions

/* Get write driver from cdrecord.ini */
driver=SysIni(installDir"\cdrecord.ini",'cdrdao','driver')
if driver='ERROR:' then exit
driver=LEFT(driver,LENGTH(driver)-1)
SAY 'Driver: 'driver

/* Get path to cdrdao from cdrecord.ini */
cdrdaoPath=SysIni(installDir"\cdrecord.ini",'cdrdao','path')
if cdrdaoPath='ERROR:' then exit
cdrdaoPath=LEFT(cdrdaoPath,LENGTH(cdrdaoPath)-1)
SAY 'cdrdaoPath: 'cdrdaoPath

/* Get default speed */
speed=SysIni(installDir"\cdrecord.ini",'device','speed')
speed=LEFT(speed,LENGTH(speed)-1)

SAY ''
SAY '-------------------------------------------------'
SAY''

/* Build cdrdao command line */
cdrdaoPath=cdrdaoPath||' copy --device 'targetDevice' --speed 'speed
if simulate=1 then cdrdaoPath=cdrdaoPath||' --simulate'     /* Simulate? */
cdrdaoPath=cdrdaoPath||' --buffers 90 --driver 'driver' 'cdrdaoOptions' --datafile 'imageDrive'/cd.bin 'imageDrive'/cd.toc'

/* Start cdrdao */
cdrdaoPath

'del 'imageDrive'\cd.bin'
'del 'imageDrive'\cd.toc'

say ''
SAY '-------------------------------------------------'
SAY ''
SAY 'Done!'

