SetPriority with exec (SPE)
===========================

SPE is a 'light' version of the sp.exe program (sp103.zip).  SP uses
a C system() call to run programs at a given priority level, and
hence a command processor is run at that priority level which then
runs the target program.  This is a resource heavy operation that
leaves a command processor running (doing nothing) until the target
program terminates, but allows SP to run any sort of OS/2 program
(OS/2, DOS or Windows) or command files.  SPE can only run OS/2 *.exe
programs, but uses a C execv() call and hence does not require a
command processor.

Usage:
   Usage:\n"
      spe <class>[delta] <program> [args...]

      <class>:  n=no change, i=idle, r=regular, f=foreground,
         t=time critical
      [delta]:  Any number from -31 to 31
      <program>: Name of the program to be run.  This should include
         the extension and can include the path.
      [args...]: The command line arguments for the <program>.

      Note that there is no whitespace between the <class> and the
      optional [delta].

Freeware, source code included (Watcom C/C++ 10.0a compiler).

Author: J S Worthington (stephen@inisant.actrix.gen.nz)
