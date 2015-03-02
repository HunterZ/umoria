/* ibmpc/ms_misc.c: MSDOS support code

   Copyright (c) 1989-94 James E. Wilson, Don Kneller

   This file is part of Umoria.

   Umoria is free software; you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Umoria is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with Umoria.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef __TURBOC__
#include <conio.h>
#endif /* __TURBOC__ */

#ifdef _MSC_VER
#include <pdcwin.h> // this brings in Sleep() from Windows.h without generating warnings
#define strcmpi _strcmpi // eliminates an ISO error
#endif

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#ifdef MSDOS
#ifndef USING_TCIO
/* We don't want to include curses.h when using the tcio.c file.  */
#include <curses.h>
#endif
#ifdef ANSI
#include "ms_ansi.h"
#endif

#ifdef LINT_ARGS
void exit(int);
static FILE *fopenp(char *, char *, char *);
static unsigned int ioctl(int, int, unsigned int);
#else
void exit();
static unsigned int ioctl();
#endif

extern char *getenv();

#define PATHLEN 80
char moriatop[PATHLEN];
char moriasav[PATHLEN];
int  saveprompt = TRUE;
#ifndef _MSC_VER
int ibmbios;
static int rawio;
#endif
int8u floorsym = '.';
int8u wallsym = '#';

/* UNIX compatability routines */
void user_name(buf)
char *buf;
{
  strcpy(buf, getlogin());
}

char*
getlogin()
{
  char* cp;

  if ((cp = getenv("USER")) == NULL)
    cp = "player";
  return cp;
}

#ifdef _MSC_VER
/* CPU-friendly sleep */
unsigned int sleep(unsigned int secs)
{
  Sleep(secs * 1000);
  return 0;
}
#elif !defined(__TURBOC__)
unsigned int
sleep(secs)
int secs;
{
  time_t finish_time;

  finish_time = time((long *) NULL) + secs;
  while (time((long *) NULL) < finish_time)
    ; /* nothing */
  return 0;
}
#endif

#if 0
/* This is the old code.  It is not strictly correct; it is retained in
   case the correct code below is not portable.  You will also have to
   change the declarations for these functions in externs.h.  */
void
error(fmt, a1, a2, a3, a4)
char *fmt;
int a1, a2, a3, a4;
{
  fprintf(stderr, "MORIA error: ");
  fprintf(stderr, fmt, a1, a2, a3, a4);
  (void) sleep(2);
  exit(1);
}

void
warn(fmt, a1, a2, a3, a4)
char *fmt;
int a1, a2, a3, a4;
{
  fprintf(stderr, "MORIA warning: ");
  fprintf(stderr, fmt, a1, a2, a3, a4);
  (void) sleep(2);
}
#else

#include <stdarg.h>

void
error (char *fmt, ...)
{
  va_list p_arg;

  va_start (p_arg, fmt);
  fprintf (stderr, "Moria error: ");
  vfprintf (stderr, fmt, p_arg);
  sleep (2);
  exit (1);
}

void
warn(char *fmt, ...)
{
  va_list p_arg;

  va_start(p_arg, fmt);
  fprintf(stderr, "MORIA warning: ");
  vfprintf(stderr, fmt, p_arg);
  sleep(2);
}
#endif

/* Search the path for a file of name "name".  The directory is
 * filled in with the directory part of the path.
 */
static FILE *
fopenp(name, mode, directory)
char *name, *mode, directory[];
{
  char *dp, *pathp, *getenv(), lastch;
  FILE *fp;

  /* Try the default directory first.  If the file can't be opened,
   * start looking along the path.
   */
  fp = fopen (name, mode);
  if (fp) {
    directory[0] = '\0';
    return fp;
  }
  pathp = getenv("PATH");
  while (pathp && *pathp) {
    dp = directory;
    while (*pathp && *pathp != ';')
      lastch = *dp++ = *pathp++;
    if (lastch != '\\' && lastch != '/' && lastch != ':')
      *dp++ = '\\';
    (void) strcpy(dp, name);
    fp = fopen (directory, mode);
    if (fp) {
      *dp = '\0';
      return fp;
    }
    if (*pathp)
      pathp++;
  }
  directory[0] = '\0';
  return NULL;
}

/* Read the configuration.
 */
void
msdos_init()
{
  char buf[BUFSIZ], *bp, opt[PATHLEN];
  int arg1, arg2, cnt, line = 0;
  FILE *fp;

  buf[0] = '\0';
  bp = MORIA_CNF_NAME;
  fp = fopenp(bp, "r", buf);
  (void) strcpy(moriatop, buf);
  (void) strcat(moriatop, MORIA_TOP_NAME);
  (void) strcpy(moriasav, buf);
  (void) strcat(moriasav, MORIA_SAV_NAME);
  if (fp == NULL) {
    warn("Can't find configuration file `%s'\n", bp);
    return;
  }
  printf("Reading configuration from %s%s\n", buf, bp);
  (void) sleep(1);
  while (fgets(buf, sizeof buf, fp)) {
    ++line;
    if (*buf == '#')
      continue;

    cnt = sscanf(buf, "%s", opt);
    /* Turbo C will return EOF when reading an empty line,
       MSC will correctly read a NULL character */
    if (cnt == 0 ||
#if defined(__TURBOC__) || defined(_MSC_VER)
        cnt == EOF ||
#endif
        opt[0] == '\0')
      continue;

    /* Go through possible variables */
    if (strcmpi(opt, "GRAPHICS") == 0) {
      cnt = sscanf(buf, "%*s%d %d\n", &arg1, &arg2);
      if (cnt != 2)
        warn("Line %d: GRAPHICS did not contain 2 values\n", line);
      else {
        wallsym = (int8u) arg1;
        floorsym = (int8u) arg2;

        /* Adjust lists that depend on '#' and '.' */
        object_list[OBJ_SECRET_DOOR].tchar = wallsym;
      }
    }
    else if (strcmpi(opt, "SAVE") == 0) {
      cnt = sscanf(buf, "%*s%s", opt);
      if (cnt == 0)
        warn("Line %d: SAVE option requires a filename\n", line);
      else {
              bp = strchr (opt, ';');
        if (bp) {
          *bp++ = '\0';
          if (*bp == 'n' || *bp == 'N')
            saveprompt = FALSE;
        }
        if (opt[0])
          (void) strcpy(moriasav, opt);
      }
    }
    else if (strcmpi(opt, "SCORE") == 0) {
      cnt = sscanf(buf, "%*s%s", opt);
      if (cnt == 0)
        warn("Line %d: SCORE option requires a filename\n", line);
      else
        (void) strcpy(moriatop, opt);
    }
    else if (strcmpi(opt, "KEYBOARD") == 0) {
      cnt = sscanf(buf, "%*s%s", opt);
      if (cnt == 0)
        warn("Line %d: KEYBOARD option requires a value\n", line);
      else if (strcmpi(opt, "ROGUE") == 0)
        rogue_like_commands = TRUE;
      else if (strcmpi(opt, "VMS") == 0)
        rogue_like_commands = FALSE;
    }
#ifndef _MSC_VER
    else if (strcmpi(opt, "IBMBIOS") == 0)
      ibmbios = TRUE;
    else if (strcmpi(opt, "RAWIO") == 0)
      rawio = TRUE;
#endif
#ifdef ANSI
    /* Usage: ANSI [ check_ansi [ domoveopt [ tgoto ] ] ]
     * where check_ansi and domoveopt are "Y"es unless explicitly
     * set to "N"o. Tgoto is "N"o unless set to "Y"es.
     */
    else if (strcmpi(opt, "ANSI") == 0) {
      cnt=sscanf(buf, "%*s%1s%1s%1s",&opt[0],&opt[1],&opt[2]);
      ansi_prep(cnt < 1 || opt[0] == 'y' || opt[0] == 'Y',
         cnt < 2 || opt[1] == 'y' || opt[1] == 'Y',
         cnt >= 3 && (opt[2] == 'y' || opt[2] == 'Y'));
    }
#endif
    else
      warn("Line %d: Unknown configuration line: `%s'\n", line, buf);
  }
  fclose(fp);

  /* The only text file has been read.  Switch to binary mode */
}

#ifdef _MSC_VER
void msdos_raw() { raw(); }
void msdos_noraw() { noraw(); }
#else
#include <dos.h>
#define DEVICE  0x80
#define RAW     0x20
#define IOCTL   0x44
#define STDIN   fileno(stdin)
#define STDOUT  fileno(stdout)
#define GETBITS 0
#define SETBITS 1

static unsigned old_stdin, old_stdout, ioctl();

void
msdos_raw() {
  if (!rawio)
    return;
  old_stdin = ioctl(STDIN, GETBITS, 0);
  old_stdout = ioctl(STDOUT, GETBITS, 0);
  if (old_stdin & DEVICE)
    ioctl(STDIN, SETBITS, old_stdin | RAW);
  if (old_stdout & DEVICE)
    ioctl(STDOUT, SETBITS, old_stdout | RAW);
}

void
msdos_noraw() {
  if (!rawio)
    return;
  if (old_stdin)
    (void) ioctl(STDIN, SETBITS, old_stdin);
  if (old_stdout)
    (void) ioctl(STDOUT, SETBITS, old_stdout);
}

static unsigned int
ioctl(handle, mode, setvalue)
unsigned int setvalue;
{
#ifdef _MSC_VER
    return 0;
#else
  union REGS regs;

  regs.h.ah = IOCTL;
  regs.h.al = (unsigned char) mode;
  regs.x.bx = handle;
  regs.h.dl = (unsigned char) setvalue;
  regs.h.dh = 0; /* Zero out dh */
  intdos(&regs, &regs);
  return (regs.x.dx);
#endif
}
#endif

/* Normal characters are output when the shift key is not pushed.
 * Shift characters are output when either shift key is pushed.
 */
#ifndef _MSC_VER
#define KEYPADHI  83
#define KEYPADLOW 71
#define ISKEYPAD(x) (KEYPADLOW <= (x) && (x) <= KEYPADHI)
#endif
#undef CTRL
#define CTRL(x) (x - '@')
typedef struct {
  char normal, shift, numlock;
} KEY;
#ifdef _MSC_VER
static const KEY roguekeypad[] = {
#else
static KEY roguekeypad[KEYPADHI - KEYPADLOW + 1] = {
#endif
  {'y', 'Y', CTRL('Y')},             /* 7 */
  {'k', 'K', CTRL('K')},             /* 8 */
  {'u', 'U', CTRL('U')},             /* 9 */
#ifdef _MSC_VER
  /* useful for config menu, if nothing else */
  {'-', '-', '-'},                   /* - */
#else
  {'.', '.', '.'},                   /* - */
#endif
  {'h', 'H', CTRL('H')},             /* 4 */
  {'.', '.', '.'},                   /* 5 */
  {'l', 'L', CTRL('L')},             /* 6 */
  {CTRL('P'), CTRL('P'), CTRL('P')}, /* + */
  {'b', 'B', CTRL('B')},             /* 1 */
  {'j', 'J', CTRL('J')},             /* 2 */
  {'n', 'N', CTRL('N')},             /* 3 */
  {'i', 'i', 'i'},                   /* Ins */
  {'.', '.', '.'}                    /* Del */
};
#ifdef _MSC_VER
static const KEY originalkeypad[] = {
#else
static KEY originalkeypad[KEYPADHI - KEYPADLOW + 1] = {
#endif
  {'7', '7', '7'},                   /* 7 */
  {'8', '8', '8'},                   /* 8 */
  {'9', '9', '9'},                   /* 9 */
  {'-', '-', '-'},                   /* - */
  {'4', '4', '4'},                   /* 4 */
  {'5', '5', '5'},                   /* 5  - move */
  {'6', '6', '6'},                   /* 6 */
  {CTRL('M'), CTRL('M'), CTRL('M')}, /* + */
  {'1', '1', '1'},                   /* 1 */
  {'2', '2', '2'},                   /* 2 */
  {'3', '3', '3'},                   /* 3 */
  {'i', 'i', 'i'},                   /* Ins */
  {'.', '.', '.'}                    /* Del */
};

#ifdef _MSC_VER
typedef enum { KEY_MOD_NORMAL, KEY_MOD_SHIFT, KEY_MOD_NUMLOCK } KEY_MOD;

/* use PDCurses keypad handling */
int pdcurses_getch()
{
    int ch = 0; /* return value for non-keypad keys */
    int keypad_index = -1; /* index into keypad arrays */
    unsigned long modifier_mask = 0; /* PDC_get_key_modifiers() value */
    const KEY* kp = (rogue_like_commands ? roguekeypad : originalkeypad); /* pointer to active array */

    /* enable PDC_KEY_MODIFIER_* return values for PDC_get_key_modifiers() */
    PDC_save_key_modifiers(TRUE);

    /* enable special return values for keypad keys */
    keypad(stdscr, TRUE);

    /* get input */
    ch = getch();

    /* save modifier mask */
    modifier_mask = PDC_get_key_modifiers();

    /* translate keypad inputs to equivalent key inputs */
    switch(ch)
    {
        case KEY_DOWN:      keypad_index = 9;  break;
        case KEY_UP:        keypad_index = 1;  break;
        case KEY_LEFT:      keypad_index = 4;  break;
        case KEY_RIGHT:     keypad_index = 6;  break;
        case KEY_HOME:      keypad_index = 0;  break;
        case KEY_DC:        keypad_index = 12; break;
        case KEY_IC:        keypad_index = 11; break;
        case KEY_NPAGE:     keypad_index = 10; break;
        case KEY_PPAGE:     keypad_index = 2;  break;
        case KEY_END:       keypad_index = 8;  break;
        case KEY_SDC:       keypad_index = 12; break;
        case KEY_SEND:      keypad_index = 8;  break;
        case KEY_SHOME:     keypad_index = 0;  break;
        case KEY_SIC:       keypad_index = 11; break;
        case KEY_SLEFT:     keypad_index = 4;  break;
        case KEY_SNEXT:     keypad_index = 10; break;
        case KEY_SPREVIOUS: keypad_index = 2;  break;
        case KEY_SRIGHT:    keypad_index = 6;  break;
        case CTL_LEFT:      keypad_index = 4;  break;
        case CTL_RIGHT:     keypad_index = 6;  break;
        case CTL_PGUP:      keypad_index = 2;  break;
        case CTL_PGDN:      keypad_index = 10; break;
        case CTL_HOME:      keypad_index = 0;  break;
        case CTL_END:       keypad_index = 8;  break;
        case KEY_A1:        keypad_index = 0;  break;
        case KEY_A2:        keypad_index = 1;  break;
        case KEY_A3:        keypad_index = 2;  break;
        case KEY_B1:        keypad_index = 4;  break;
        case KEY_B2:        keypad_index = 5;  break;
        case KEY_B3:        keypad_index = 6;  break;
        case KEY_C1:        keypad_index = 8;  break;
        case KEY_C2:        keypad_index = 9;  break;
        case KEY_C3:        keypad_index = 10; break;
        case PADSTOP:       keypad_index = 12; break;
        case PADMINUS:      keypad_index = 3;  break;
        case PADPLUS:       keypad_index = 7;  break;
        case CTL_PADSTOP:   keypad_index = 12; break;
        case CTL_PADPLUS:   keypad_index = 7;  break;
        case CTL_PADMINUS:  keypad_index = 3;  break;
        case CTL_INS:       keypad_index = 11; break;
        case CTL_UP:        keypad_index = 1;  break;
        case CTL_DOWN:      keypad_index = 9;  break;
        case PAD0:          keypad_index = 11; break;
        case CTL_PAD0:      keypad_index = 11; break;
        case CTL_PAD1:      keypad_index = 8;  break;
        case CTL_PAD2:      keypad_index = 9;  break;
        case CTL_PAD3:      keypad_index = 10; break;
        case CTL_PAD4:      keypad_index = 4;  break;
        case CTL_PAD5:      keypad_index = 5;  break;
        case CTL_PAD6:      keypad_index = 6;  break;
        case CTL_PAD7:      keypad_index = 0;  break;
        case CTL_PAD8:      keypad_index = 1;  break;
        case CTL_PAD9:      keypad_index = 2;  break;
        case CTL_DEL:       keypad_index = 12; break;
        case SHF_PADPLUS:   keypad_index = 7;  break;
        case SHF_PADMINUS:  keypad_index = 3;  break;
        case KEY_SUP:       keypad_index = 1;  break;
        case KEY_SDOWN:     keypad_index = 9;  break;
        default:
            ;
    }

    if (keypad_index == -1 &&
        (modifier_mask & PDC_KEY_MODIFIER_SHIFT ||
         modifier_mask & PDC_KEY_MODIFIER_NUMLOCK))
    {
        /* special case: if shift+number is pressed,
           then it's actually shift+keypad */
        /* also err on the side of interpreting numbers
           as keypad keys when numlock is enabled */
        switch (ch)
        {
            case '.': keypad_index = 12; break;
            case '0': keypad_index = 11; break;
            case '1': keypad_index = 8;  break;
            case '2': keypad_index = 9;  break;
            case '3': keypad_index = 10; break;
            case '4': keypad_index = 4;  break;
            case '5': keypad_index = 5;  break;
            case '6': keypad_index = 6;  break;
            case '7': keypad_index = 0;  break;
            case '8': keypad_index = 1;  break;
            case '9': keypad_index = 2;  break;
            default:
                ;
        }
    }

    /* handle modifiers on keypad keys
       I could have gleaned this from key codes in many cases,
       but it saves code to just do check modifiers separately */
    if (keypad_index >= 0)
    {
        /* treating CTRL as equivalent to NUMLOCK
           allows CTRL+direction to work per the in-game help */
        if (modifier_mask & PDC_KEY_MODIFIER_NUMLOCK ||
            modifier_mask & PDC_KEY_MODIFIER_CONTROL)
        {
            return kp[keypad_index].numlock;
        }
        else if (modifier_mask & PDC_KEY_MODIFIER_SHIFT)
        {
            return kp[keypad_index].shift;
        }
        else
        {
            return kp[keypad_index].normal;
        }
    }

    /* special case: handle additional keypad keys */
    switch (ch)
    {
        case PADSLASH:     return '/'; break;
        case PADENTER:     return CTRL('M'); break;
        case CTL_PADENTER: return CTRL('M'); break;
        case ALT_PADENTER: return CTRL('M'); break;
        case PADSTAR:      return '*'; break;
        case CTL_PADSLASH: return '/'; break;
        case CTL_PADSTAR:  return '*'; break;
        case ALT_PADSLASH: return '/'; break;
        case ALT_PADSTAR:  return '*'; break;
        case SHF_PADENTER: return CTRL('M'); break;
        case SHF_PADSLASH: return '/'; break;
        case SHF_PADSTAR:  return '*'; break;
        default:
            ;
    }

    return ch;
}
#else
/* bios_getch gets keys directly with a BIOS call.
 */
#define SHIFT       (0x1 | 0x2)
#define NUMLOCK     0x20
#define KEYBRD_BIOS 0x16

int
bios_getch()
{
  unsigned char scan, shift;
  int ch;
  KEY *kp;
  union REGS regs;

  if (rogue_like_commands)
    kp = roguekeypad;
  else
    kp = originalkeypad;

  /* Get scan code.
   */
  regs.h.ah = 0;
  int86(KEYBRD_BIOS, &regs, &regs);
  ch = regs.h.al;
  scan = regs.h.ah;

  /* Get shift status.
   */
  regs.h.ah = 2;
  int86(KEYBRD_BIOS, &regs, &regs);
  shift = regs.h.al;

  /* If scan code is for the keypad, translate it.
   */
  if (ISKEYPAD(scan)) {
    if (shift & NUMLOCK)
      ch = kp[scan - KEYPADLOW].numlock;
    else if (shift & SHIFT)
      ch = kp[scan - KEYPADLOW].shift;
    else
      ch = kp[scan - KEYPADLOW].normal;
  }
  return ch;
}
#endif

int
msdos_getch()
{
#ifdef _MSC_VER
    return pdcurses_getch();
#else
    int ch;
    if (ibmbios)
    ch = bios_getch();
  else
    {
    ch = getch();
    if (ch == 0)
      ch = getch();
  }
  return ch;
#endif
}

#if 0
/* This intro message deleted because it is obsolete.  */

/* Hardcode the introductory message in */
void
msdos_intro()
{
        char buf[80];

  clear_screen();
        wmove(stdscr,0,0);
  waddstr(stdscr,"                         *********************");
  wmove(stdscr,1,0);
        sprintf(buf,"                         **   Moria %d.%d    **",
    CUR_VERSION_MAJ, CUR_VERSION_MIN);
        waddstr(stdscr,buf);
        wmove(stdscr,2,0);
  waddstr(stdscr,"                         *********************");
        wmove(stdscr,3,0);
  waddstr(stdscr,"                   COPYRIGHT (c) Robert Alan Koeneke");
        wmove(stdscr,5,0);
  waddstr(stdscr,"Programmers : Robert Alan Koeneke / University of Oklahoma");
        wmove(stdscr,6,0);
  waddstr(stdscr,"              Jimmey Wayne Todd   / University of Oklahoma");
        wmove(stdscr,8,0);
  waddstr(stdscr,"UNIX Port   : James E. Wilson     / Cygnus Support");
        wmove(stdscr,10,0);
  waddstr(stdscr,"MSDOS Port  : Don Kneller         / 1349 - 10th ave");
        wmove(stdscr,11,0);
  waddstr(stdscr,
    "                                  / San Francisco, CA 94122");
        wmove(stdscr,12,0);
  waddstr(stdscr,"                                  / Dec 12, 1988");
  pause_line(23);
}
#endif

#ifdef PC_CURSES
/* Seems to be a bug in PCcurses whereby it won't really clear the screen
 * if there are characters there it doesn't know about.
 */
#define VIDEOINT 0x10
void
bios_clear()
{
  union REGS regs;
  unsigned char nocols, activepage;

#ifdef ANSI
  if (ansi)
    return;
#endif

  /* get video attributes */
  regs.h.ah = 15;
  int86(VIDEOINT, &regs, &regs);
  nocols = regs.h.ah;
  activepage = regs.h.bh;

  /* Move to lower right corner */
  regs.h.ah = 2;
  regs.h.dh = (unsigned char) 24;
  regs.h.dl = nocols - 1; /* lower right col */
  regs.h.bh = activepage;
  int86(VIDEOINT, &regs, &regs);

  /* get current attribute into bh */
  regs.h.ah = 8;
  regs.h.bh = activepage;
  int86(VIDEOINT, &regs, &regs);
  regs.h.bh = regs.h.ah;

  regs.h.cl = 0; /* upper left row */
  regs.h.ch = 0; /* upper left col */
  regs.h.dh = (unsigned char) 24; /* lower right row */
  regs.h.dl = nocols - 1; /* lower right col */
  regs.h.al = 0; /* clear window */
  regs.h.ah = 7; /* scroll down */
  int86(VIDEOINT, &regs, &regs);
}
#endif /* PC_CURSES */
#endif /* MSDOS */
