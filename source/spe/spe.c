/*****************************************************************************
	SetPriority with exec (SPE)

	Author:
		J S Worthington
		98B Waiwhetu Road
		Lower Hutt
		New Zealand
		Phone:
			+64-4-569-6764 (home)
			+64-4-389-8909 (work)
		Fax:
			+64-4-389-9901 (work)
		Web:
			http://www.digitech.co.nz (work)
		Email:
			stephen@inisant.actrix.gen.nz (home)
			stephen@digitech.co.nz (work)

	Based on the sp.c source code in sp102.zip by Jens Glathe:
		Angepa·t an OS/2 Version 2.x von Jens Glathe, 3.9.1993
		Erweiterung fÅr das Beeinflussen von DOS-Boxen: 6.9.1993,16/17.9.1993
		letzte énderung: Jens Glathe 15.05.1994
		SetPriority Version 1.02 (c) 1993, 1994 Jens Glathe, Scott Dudley

	I wrote this because the original sp program uses system() to execute the
	program.  This causes a command processor to be used to run the program,
	and the command processor remains in use (but not executing) until the
	executed program exits.  This is a waste of resources.  By using exec(),
	this version only leaves the executed program running.
*****************************************************************************/

#define INCL_DOSPROCESS
#define INCL_NOPM
#include <os2.h>

#include <ctype.h>
#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

/*****************************************************************************
	Macros.
*****************************************************************************/

#define array_size(array) \
	(sizeof (array) / sizeof (array[0]))

#define for_all(ptr, array) \
	for (ptr = array; ptr < &array[array_size(array)]; ++ptr)


/*****************************************************************************
	Type definitions.
*****************************************************************************/

/* Priority class table entry. */
typedef struct {
	char
		command_char;			/* The command line character that selects the
										required priority class. */
	USHORT
		priority_class;		/* The priority class. */
	char
		*description;			/* String describing the priority class. */
} PRIORITY_CLASS;


/*****************************************************************************
	Static data.
*****************************************************************************/

/* The priority class table. */
static PRIORITY_CLASS priority_class_table[] = {
	{'n', PRTYC_NOCHANGE, "no change in"},
	{'i', PRTYC_IDLETIME, "idle"},
	{'r', PRTYC_REGULAR, "regular"},
	{'t', PRTYC_TIMECRITICAL, "time critical"},
	{'f', PRTYC_FOREGROUNDSERVER, "foreground"},
};

/* Pointer to the current priority class. */
static PRIORITY_CLASS
	*pcp;

static char
	priority_char;			/* Command line character used to select the priority
									class. */

static LONG
	priority_delta = 0;

static APIRET
	rc;						/* OS/2 API return code. */


/*****************************************************************************
	Error - display the usage message and exit.
*****************************************************************************/

void usage(void) {
	printf(
		"Usage:\n"
		"   spe <class>[delta] <program> [args...]\n\n"
		"   <class>:  n=no change, i=idle, r=regular, f=foreground,"
			"t=time critical\n"
		"   [delta]:  Any number from -31 to 31\n"
      "   <program>: Name of the program to be run.  This should include\n"
      "      the extension and can include the path.\n"
      "   [args...]: The command line arguments for the <program>.\n"
		"\n"
      "   Note that there is no whitespace between the <class> and the\n"
      "   optional [delta].\n"
	);
	exit(1);
}


/*****************************************************************************
	Main program.
*****************************************************************************/

int main(
	int
		argc,
	char
		*argv[]
) {
	if (argc < 3) {
		usage();
	}

	priority_char = (char)tolower(*argv[1]);

	for_all(pcp, priority_class_table) {
		if (pcp->command_char == priority_char) {
			break;
		}
	}
	if (pcp >= &priority_class_table[array_size(priority_class_table)]) {
		usage();
	}

	priority_delta = atoi(argv[1] + 1);

	if (priority_delta < -31 || priority_delta > 31) {
		usage();
	}

	printf(
		"SPE: Running %s at %s priority, delta %ld\n",
		argv[2],
		pcp->description,
		priority_delta
	);

	if (
		(rc = DosSetPriority(
			PRTYS_PROCESSTREE,
			pcp->priority_class,
			priority_delta,
			0UL
		)) != 0
	) {
		printf("SPE: DosSetPriority() rc = %lu\n", rc);
		return rc;
	}

	(void)execv(argv[2], &argv[2]);

	/* Since execv() will not return unless it found an error and could not
		execute the program, if we get here we must have an error. */

	printf("SPE: execv() errno = %d\n", errno);
	return errno;
}
