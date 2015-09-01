This is UMoria 5.6, compiled for 32-bit Windows using Cygwin's i686-w64-mingw32-gcc package + git PDCurses.

The files in this folder are necessary to run UMoria, while the documentation is in the doc subfolder.

To run the game, launch moria.exe. Some configuration options can be found in moria.cnf.

I was recently inspired to produce modern Windows ports of UMoria 5.6 using Cygwin and Microsoft Visual Studio 2013, with the latter port intended to provide an experience closer to that of the old MS-DOS ports. Unfotunately, the VS2013 port doesn't seem to work in Windows XP, so I decided to produce this build from the same source but using MinGW (a Windows port of GCC).

To the above end, this port uses the MS-DOS flavor of the source code as a baseline. However, I had to make a number of changes to replicate the I/O functionality using PDCurses instead of ANSI or low-level MS-DOS I/O facilities that are not available in modern Windows. There were a number of challenges involved, including translation from DOS/Extended ASCII (code page 437) to Unicode in order to support the graphics characters used by the MS-DOS port, and translation of the keypad key codes from PDCurses to equivalent actions from the MS-DOS port. It is my hope that fans of the MS-DOS versions will find this to be a worthy update to Microsoft's current family of operating systems.

Any suspected bugs in UMoria should be reported to the maintainer, David Grabiner. Be sure to include version/platform information with your report.

Official UMoria source code, plus source code and binary distributions for this port may be found on Github at https://github.com/HunterZ/umoria

Ben Shadwick
benshadwick@gmail.com
