#ifndef CP437UTF_H
#define CP437UTF_H

/* include PDCurses header for cchar_t support */
# ifdef _MSC_VER /* Visual Studio */
#  include <curses.h>
# else
#  include <pdcurses.h> /* MinGW32 etc. */
# endif

/* wide character (16-bit UTF) equivalents for OEM extended ASCII / MS-DOS
   character set (8-bit code page 437)
   source: http://en.wikipedia.org/wiki/Code_page_437 */
extern const cchar_t cp437utf[256];

/* convert CP437 (IBM PC) character to PDCurses cchar_t wide character
   equivalent, preserving any attributes
   this is probably somewhat bogus, but doesn't seem to hurt anything */
cchar_t ToUTF(const int c);

#endif
