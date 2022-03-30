This is UMoria 5.6, compiled for 64-bit Windows using MSYS2 MinGW64 + PDCurses.

The files in this folder are necessary to run UMoria, while the documentation is in the doc subfolder.

To run the game, launch umoria.exe. Some configuration options can be found in MORIA.CNF.

I attempted to produce modern Windows ports of 5.6 using Cygwin and Visual Studio several years ago, but was never quite happy with them, so here I go again with MinGW!

This port uses the MS-DOS (and MSVC) flavor of the source code as a baseline. However, I had to port the I/O code to use PDCurses instead of ANSI and/or low-level MS-DOS I/O facilities that are not available in modern Windows. There were a number of challenges involved, including translation from DOS/Extended ASCII (code page 437) to Unicode in order to support the graphics characters used by the MS-DOS port, and translation of the keypad key codes from PDCurses to equivalent actions from the MS-DOS port. It is my hope that fans of the MS-DOS versions will find this to be a worthy update to Microsoft's current family of operating systems.

Official UMoria source code, plus source code and binary distributions for this port may be found on Github at https://github.com/HunterZ/umoria

Please note that support will not be provided for any bugs/issues in the original game code, as this port is meant to reflect the state of functionality of the last official release by the last official maintainer, David Grabiner. Potential port-specific issues may be reported on GitHub, however.

Ben Shadwick
benshadwick@gmail.com
