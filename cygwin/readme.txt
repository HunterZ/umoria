This is UMoria 5.6, compiled for 32-bit Windows using Cygwin GCC+ncurses.

The files in this directory are necessary to run UMoria, while the
documentation is in the doc subdirectory.

To run without Cygwin, double-click moria.bat. To run inside of Cygwin,
run ./moria.exe directly. In Cygwin, save files will be written to your
Cygwin home directory by default, while in regular Windows they will be
written to whatever the HOME variable is set to in moria.bat (which
defaults to the directory from which the game is run, but you can change
this to, say, 'set HOME=%USERPROFILE%' to save to your Windows user folder
if you want to run the game from a read-only location).

I decided to use Cygwin to produce this port because UMoria's source code
contains a lot of Unix/POSIX-isms. Cygwin is ideal here, because its entire
purpose is to provide a POSIX compatbility layer on Windows. Other than a
fix for modern ncurses and #ifdef'ing out a signal that doesn't exist in
Cygwin, this port follows the Linux flavor of the code very closely.

Since this is a port of the Unix flavor of the source code, it does not
contain any of the MS-DOS flavor's enhancements that are in the UMoria
5.5.2 32-bit MS-DOS version that I produced many years ago. As a result,
some people may find it preferable to continue using that version via
DOSBox. On the other hand, this version is a port of the 5.6 source code,
which contains additional fixes by David Grabiner. It should also be more
or less feature equivalent to the Debian Linux distribution of the game.

Any suspected bugs in UMoria should be reported to the maintainer, David
Grabiner. Be sure to include version/platform information with your report.

Official UMoria source code, plus source code and binary distributions for
this port may be found on Github at https://github.com/HunterZ/umoria

Ben Shadwick
benshadwick@gmail.com

