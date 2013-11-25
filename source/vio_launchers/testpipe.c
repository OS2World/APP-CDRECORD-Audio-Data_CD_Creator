/*
 * This file is (C) Chris Wohlgemuth 1999-2003
 *
 * This helper handles the image writing
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
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN

#include <os2.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>


void message()
{
  printf("This program is (C) Chris Wohlgemuth 1999-2003\n");
  printf("See the file COPYING for the license.\n\n");
  printf("This program should only be called by the CD-Creater WPS classes.\n");
}


/*
  argv[0]: progname
  argv[1]: text file
 */

int main(int argc, char * argv[])
{
  FILE *file;
  char buf[100];

  if(argc!=2)
    {
      message();
      exit(2);
    }
  file=fopen(argv[1], "r");

  if(!file)
    {
      printf("ERROR:\n");
      exit(1);
    }

  do {
    fseek(file, 0, SEEK_SET);
    DosSleep(1000);
  }
  while(!fgets(buf,sizeof(buf),file));


  while(!fseek(file, -10, SEEK_END) && fgets(buf,sizeof(buf),file))
    {
      char *chrPtr;

      chrPtr=strrchr(buf, '\n');
      if(chrPtr)
        {
          *chrPtr=0;
        }
      //      printf(strlwr(buf));

      /* User killed the grab process */
      if(strstr(strlwr(buf), "with 2"))
        {
          printf("DONE\n");
          break;
        }
        
      chrPtr=strrchr(buf, '%');
      if(chrPtr)
        {
          *chrPtr=0;
          printf(buf);
          printf("\n");
        }
      else {
        if(strstr(strlwr(buf), "recorded"))
          {
            printf("DONE\n");
            break;
          } 
      }
      DosSleep(1000);
    }
  fclose(file);
}









