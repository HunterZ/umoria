/* windows/ms_misc.c: Windows support code by Ben Shadwick

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

#ifdef _MSC_VER
# include <curses.h>
#else
# include <pdcurses.h>
#endif

#ifdef _MSC_VER
# include <pdcwin.h> // this brings in Sleep() from Windows.h without generating warnings
# define strcmpi _strcmpi // eliminates an ISO error
#else
# include <stdlib.h> /* getenv() */
# include <unistd.h> /* sleep() */
#endif

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#ifndef __MINGW32__
void exit();
static unsigned int ioctl();
extern char *getenv();
#endif

#define PATHLEN 260 /* was 80, but windows supports 260 */
char moriatop[PATHLEN];
char moriasav[PATHLEN];
int  saveprompt = TRUE;
int8u floorsym = '.';
int8u wallsym = '#';

/* UNIX compatability routines */
void user_name(char* buf)
{
  strcpy(buf, getlogin());
}

char* getlogin()
{
  /* try to only do this once per run */
  static char* cp = NULL;
  if (cp == NULL)
  {
    /* try windows %USERNAME% first */
    cp = getenv("USERNAME");
    if (cp == NULL)
    {
      /* fall back to canned string */
      static char* player = "player";
      cp = player;
    }
  }

  return cp;
}

#ifdef _MSC_VER
/* CPU-friendly sleep */
unsigned int sleep(unsigned int secs)
{
  Sleep(secs * 1000);
  return 0;
}
#endif

/* this doesn't seem to be used anywhere */
void error(char *fmt, ...)
{
  va_list p_arg;

  va_start (p_arg, fmt);
  fprintf (stderr, "Moria error: ");
  vfprintf (stderr, fmt, p_arg);
  sleep(2);
  exit(1);
}

void warn(char *fmt, ...)
{
  va_list p_arg;

  va_start(p_arg, fmt);
  fprintf(stderr, "Moria warning: ");
  vfprintf(stderr, fmt, p_arg);
  sleep(2);
}

/* Search the path for a file of name "name".  The directory is
 * filled in with the directory part of the path.
 */
static FILE* fopenp(char* name, char* mode, char directory[])
{
  char *dp, *pathp, lastch;
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
void msdos_init()
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
        cnt == EOF ||
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
    else
      warn("Line %d: Unknown configuration line: `%s'\n", line, buf);
  }
  fclose(fp);

  /* The only text file has been read.  Switch to binary mode */
}

void msdos_raw() { raw(); }
void msdos_noraw() { noraw(); }

/* Normal characters are output when the shift key is not pushed.
 * Shift characters are output when either shift key is pushed.
 */
#undef CTRL
#define CTRL(x) (x - '@')
typedef char KEYPADMAP[13][3];
static const KEYPADMAP roguekeypad = {
  {'y', 'Y', CTRL('Y')},             /* 7 */
  {'k', 'K', CTRL('K')},             /* 8 */
  {'u', 'U', CTRL('U')},             /* 9 */
  {'-', '-', '-'},                   /* - */
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

static const KEYPADMAP originalkeypad = {
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
  {'0', '0', '0'},                   /* Ins */
  {'.', '.', '.'}                    /* Del */
};

/* make an enum value for each keypad row index
   this is for readability */
typedef enum
{
  KE_7 =  0,
  KE_8 =  1,
  KE_9 =  2,
  KE_M =  3,
  KE_4 =  4,
  KE_5 =  5,
  KE_6 =  6,
  KE_P =  7,
  KE_1 =  8,
  KE_2 =  9,
  KE_3 = 10,
  KE_I = 11,
  KE_D = 12
} KeypadEnum;

/* and another for each keypad column index */
typedef enum
{
  ME_NORMAL  = 0,
  ME_SHIFT   = 1,
  ME_NUMLOCK = 2
} ModifierEnum;

/* use PDCurses keypad handling */
int pdcurses_getch()
{
  int ch = 0; /* return value for non-keypad keys */
  int keypad_index = -1; /* index into keypad arrays */
  unsigned long modifier_mask = 0; /* PDC_get_key_modifiers() value */
  const KEYPADMAP* kp = (rogue_like_commands ? &roguekeypad : &originalkeypad); /* pointer to active array */
  ModifierEnum modifier = ME_NORMAL;
  char msgbuf[80];

  /* enable PDC_KEY_MODIFIER_* return values for PDC_get_key_modifiers() */
  PDC_save_key_modifiers(TRUE);

  /* enable special return values for keypad keys */
  keypad(stdscr, TRUE);

  /* get input */
  while (KEY_RESIZE == (ch = getch()))
  {
    /* attempt to enforce fixed console size
       doesn't seem to actually work though */
    resize_term(24, 80);
  }

  /* save modifier mask */
  modifier_mask = PDC_get_key_modifiers();

  /* translate keypad inputs to equivalent key inputs
     also glean what we can of modifier states */
  switch(ch)
  {
    case KEY_DOWN:      keypad_index = KE_2; modifier = ME_NORMAL;  break;
    case KEY_UP:        keypad_index = KE_8; modifier = ME_NORMAL;  break;
    case KEY_LEFT:      keypad_index = KE_4; modifier = ME_NORMAL;  break;
    case KEY_RIGHT:     keypad_index = KE_6; modifier = ME_NORMAL;  break;
    case KEY_HOME:      keypad_index = KE_7; modifier = ME_NORMAL;  break;
    case KEY_DC:        keypad_index = KE_D; modifier = ME_NORMAL;  break;
    case KEY_IC:        keypad_index = KE_I; modifier = ME_NORMAL;  break;
    case KEY_NPAGE:     keypad_index = KE_3; modifier = ME_NORMAL;  break;
    case KEY_PPAGE:     keypad_index = KE_9; modifier = ME_NORMAL;  break;
    case KEY_END:       keypad_index = KE_1; modifier = ME_NORMAL;  break;
    case KEY_SDC:       keypad_index = KE_D; modifier = ME_SHIFT;   break;
    case KEY_SEND:      keypad_index = KE_1; modifier = ME_SHIFT;   break;
    case KEY_SHOME:     keypad_index = KE_7; modifier = ME_SHIFT;   break;
    case KEY_SIC:       keypad_index = KE_I; modifier = ME_SHIFT;   break;
    case KEY_SLEFT:     keypad_index = KE_4; modifier = ME_SHIFT;   break;
    case KEY_SNEXT:     keypad_index = KE_3; modifier = ME_SHIFT;   break;
    case KEY_SPREVIOUS: keypad_index = KE_9; modifier = ME_SHIFT;   break;
    case KEY_SRIGHT:    keypad_index = KE_6; modifier = ME_SHIFT;   break;
    case CTL_LEFT:      keypad_index = KE_4; modifier = ME_NUMLOCK; break;
    case CTL_RIGHT:     keypad_index = KE_6; modifier = ME_NUMLOCK; break;
    case CTL_PGUP:      keypad_index = KE_9; modifier = ME_NUMLOCK; break;
    case CTL_PGDN:      keypad_index = KE_3; modifier = ME_NUMLOCK; break;
    case CTL_HOME:      keypad_index = KE_7; modifier = ME_NUMLOCK; break;
    case CTL_END:       keypad_index = KE_1; modifier = ME_NUMLOCK; break;
    case KEY_A1:        keypad_index = KE_7; modifier = ME_NORMAL;  break;
    case KEY_A2:        keypad_index = KE_8; modifier = ME_NORMAL;  break;
    case KEY_A3:        keypad_index = KE_9; modifier = ME_NORMAL;  break;
    case KEY_B1:        keypad_index = KE_4; modifier = ME_NORMAL;  break;
    case KEY_B2:        keypad_index = KE_5; modifier = ME_NORMAL;  break;
    case KEY_B3:        keypad_index = KE_6; modifier = ME_NORMAL;  break;
    case KEY_C1:        keypad_index = KE_1; modifier = ME_NORMAL;  break;
    case KEY_C2:        keypad_index = KE_2; modifier = ME_NORMAL;  break;
    case KEY_C3:        keypad_index = KE_3; modifier = ME_NORMAL;  break;
    case PADSTOP:       keypad_index = KE_D; modifier = ME_NORMAL;  break;
    case PADMINUS:      keypad_index = KE_M; modifier = ME_NORMAL;  break;
    case PADPLUS:       keypad_index = KE_P; modifier = ME_NORMAL;  break;
    case CTL_PADSTOP:   keypad_index = KE_D; modifier = ME_NUMLOCK; break;
    case CTL_PADPLUS:   keypad_index = KE_P; modifier = ME_NUMLOCK; break;
    case CTL_PADMINUS:  keypad_index = KE_M; modifier = ME_NUMLOCK; break;
    case CTL_INS:       keypad_index = KE_I; modifier = ME_NUMLOCK; break;
    case CTL_UP:        keypad_index = KE_8; modifier = ME_NUMLOCK; break;
    case CTL_DOWN:      keypad_index = KE_2; modifier = ME_NUMLOCK; break;
    case PAD0:          keypad_index = KE_I; modifier = ME_NORMAL;  break;
    case CTL_PAD0:      keypad_index = KE_I; modifier = ME_NUMLOCK; break;
    case CTL_PAD1:      keypad_index = KE_1; modifier = ME_NUMLOCK; break;
    case CTL_PAD2:      keypad_index = KE_2; modifier = ME_NUMLOCK; break;
    case CTL_PAD3:      keypad_index = KE_3; modifier = ME_NUMLOCK; break;
    case CTL_PAD4:      keypad_index = KE_4; modifier = ME_NUMLOCK; break;
    case CTL_PAD5:      keypad_index = KE_5; modifier = ME_NUMLOCK; break;
    case CTL_PAD6:      keypad_index = KE_6; modifier = ME_NUMLOCK; break;
    case CTL_PAD7:      keypad_index = KE_7; modifier = ME_NUMLOCK; break;
    case CTL_PAD8:      keypad_index = KE_8; modifier = ME_NUMLOCK; break;
    case CTL_PAD9:      keypad_index = KE_9; modifier = ME_NUMLOCK; break;
    case CTL_DEL:       keypad_index = KE_D; modifier = ME_NUMLOCK; break;
    case SHF_PADPLUS:   keypad_index = KE_P; modifier = ME_SHIFT;   break;
    case SHF_PADMINUS:  keypad_index = KE_M; modifier = ME_SHIFT;   break;
    case SHF_UP:        keypad_index = KE_8; modifier = ME_SHIFT;   break;
    case SHF_DOWN:      keypad_index = KE_2; modifier = ME_SHIFT;   break;
    case SHF_IC:        keypad_index = KE_I; modifier = ME_SHIFT;   break;
    case SHF_DC:        keypad_index = KE_D; modifier = ME_SHIFT;   break;
    case KEY_SUP:       keypad_index = KE_8; modifier = ME_SHIFT;   break;
    case KEY_SDOWN:     keypad_index = KE_2; modifier = ME_SHIFT;   break;
    default:
      ;
  }
  /* special case: if shift+number is pressed, assume it's actually
     shift+keypad, as this can happen in some curses implementations */
  if (keypad_index < 0 &&
      modifier_mask & PDC_KEY_MODIFIER_SHIFT)
  {
    switch (ch)
    {
      case '.': keypad_index = KE_D; break;
      case '0': keypad_index = KE_I; break;
      case '1': keypad_index = KE_1; break;
      case '2': keypad_index = KE_2; break;
      case '3': keypad_index = KE_3; break;
      case '4': keypad_index = KE_4; break;
      case '5': keypad_index = KE_5; break;
      case '6': keypad_index = KE_6; break;
      case '7': keypad_index = KE_7; break;
      case '8': keypad_index = KE_8; break;
      case '9': keypad_index = KE_9; break;
      default:
        ;
    }
  }

  /* return final determination for keypad keys */
  if (keypad_index >= 0)
  {
    /* if no modifier assigned by keyboard mapper,
       check PDCurses modifier mask just in case
       note that this is unreliable */
    if (ME_NORMAL == modifier)
    {
      switch (modifier_mask)
      {
        case PDC_KEY_MODIFIER_SHIFT:   modifier = ME_SHIFT;   break;
        case PDC_KEY_MODIFIER_CONTROL: modifier = ME_NUMLOCK; break;
        case PDC_KEY_MODIFIER_NUMLOCK: modifier = ME_NUMLOCK; break;
        default: ;
      }
    }

    return (*kp)[keypad_index][modifier];
  }

  /* special case: handle additional keypad keys */
  switch (ch)
  {
    case PADSLASH:     return '/';       break;
    case PADENTER:     return CTRL('M'); break;
    case CTL_PADENTER: return CTRL('M'); break;
//    case ALT_PADENTER: return CTRL('M'); break;
    case PADSTAR:      return '*';       break;
    case CTL_PADSLASH: return '/';       break;
    case CTL_PADSTAR:  return '*';       break;
//    case ALT_PADSLASH: return '/';       break;
//    case ALT_PADSTAR:  return '*';       break;
    case SHF_PADENTER: return CTRL('M'); break;
    case SHF_PADSLASH: return '/';       break;
    case SHF_PADSTAR:  return '*';       break;
    default:
      ;
  }

  return ch;
}

int msdos_getch()
{
  return pdcurses_getch();
}
