/* source/io.c: terminal I/O code, uses the curses package

   Copyright (C) 1989-2008 James E. Wilson, Robert A. Koeneke,
                           David J. Grabiner

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

#include <stdio.h>

#include "config.h"

#ifdef HPUX
#include <sys/bsdtty.h>
#endif

#if defined(atarist) && defined(__GNUC__)
#include <osbind.h>
#endif

#ifdef MSDOS
#include <process.h>
# ifdef __MINGW32__
#  include <unistd.h> /* sleep() */
# endif
#endif

#ifdef AMIGA
/* detach from cli process */
long _stack = 30000;
long _priority = 0;
long _BackGroundIO = 1;
char *_procname = "Moria";
#endif

#if defined(NLS) && defined(lint)
/* for AIX, don't let curses include the NL stuff */
#undef NLS
#endif

#ifdef MAC /* Mac */
# ifdef THINK_C
#  include "ScrnMgr.h"
# else
#  include <scrnmgr.h>
# endif
#elif defined(GEMDOS) /* Atari ST */
# include "curses.h"
long wgetch();
# ifdef ATARIST_TC
#  include <tos.h>	/* TC */
#  include <ext.h>
# else
#  include <osbind.h>	/* MWC */
# endif
char *getenv();
#elif defined(_MSC_VER) || defined(__MINGW32__) /* Windows */
# ifdef _MSC_VER /* Visual Studio */
#  include <curses.h>
# else
#  include <pdcurses.h> /* MinGW32 etc. */
# endif

# ifdef PDC_WIDE
#  include "cp437utf.h"
# endif
#else /* everything else (*nix etc.) */
# include <ncurses.h>
#endif

#include <ctype.h>

#if defined(SYS_V) && defined(lint)
/* for AIX, prevent hundreds of unnecessary lint errors, must define before
   signal.h is included */
#define _h_IEEETRAP
typedef struct { int stuff; } fpvmach;
#endif

#if defined(MSDOS)
#if defined(ANSI)
#include "ms_ansi.h"
#endif
#else /* not msdos */
#if !defined(ATARI_ST) && !defined(MAC) && !defined(AMIGA)
#ifndef VMS
#include <sys/ioctl.h>
#endif
#include <signal.h>
#endif
#endif

#ifndef USG
/* only needed for Berkeley UNIX */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/types.h>
#endif

#ifdef USG
#ifndef ATARI_ST
#include <string.h>
#else
#include "string.h"
#endif
#if 0
/* Used to include termio.h here, but that caused problems on some systems,
   as curses.h includes it also above.  */
#if !defined(MAC) && !defined(MSDOS) && !defined(ATARI_ST)
#if !defined(AMIGA) && !defined(VMS)
#include <termio.h>
#endif
#endif
#endif /* 0 */
#ifdef HPUX
/* Needs termio.h because curses.h doesn't include it */
#include <termio.h>
#endif
#else /* ! USG */
#include <strings.h>
#if defined(atarist) && defined(__GNUC__)
/* doesn't have sys/wait.h */
#else
#include <sys/wait.h>
#endif
#endif

#include <stdlib.h>

#ifdef DEBIAN_LINUX
#include <termios.h>
#endif

#if defined(unix) || defined(__linux__) || defined(DEBIAN_LINUX) || defined(__CYGWIN__)
#include <unistd.h> /* prototype for execl */
#endif

/* These are included after other includes (particularly curses.h)
   to avoid redefintion warnings. */

#include "constant.h"
#include "types.h"
#include "externs.h"

#if defined(SYS_V) && defined(lint)
struct screen { int dumb; };
#endif

/* Fooling lint. Unfortunately, c defines all the TIO.	  -CJS-
   constants to be long, and lint expects them to be int. Also,
   ioctl is sometimes called with just two arguments. The
   following definition keeps lint happy. It may need to be
   reset for different systems.	 */
#ifndef MAC
#ifdef lint
#ifdef Pyramid
/* Pyramid makes constants greater than 65535 into long! Gakk! -CJS- */
/*ARGSUSED*/
/*VARARGS2*/
static Ioctl(i, l, p) long l; char *p; { return 0; }
#else
/*ARGSUSED*/
/*VARARGS2*/
static Ioctl(i, l, p) char *p; { return 0; }
#endif
#define ioctl	    Ioctl
#endif

#if !defined(USG) && defined(lint)
/* This use_value hack is for curses macros which return a value,
   but don't shut up about it when you try to tell them (void).	 */
/* only needed for Berkeley UNIX */
int Use_value;
#define use_value   Use_value +=
#else
#define use_value
#endif

#if defined(SYS_V) && defined(lint)
/* This use_value2 hack is for curses macros which use a conditional
   expression, and which say null effect even if you cast to (void). */
/* only needed for SYS V */
int Use_value2;
#define use_value2  Use_value2 +=
#else
#define use_value2
#endif

#endif

#ifndef MAC
char *getenv();
#endif

#if !defined(VMS) && defined(USG)
void exit();
# if defined(__TURBOC__)
void sleep();
# elif !defined(AMIGA) && !defined(MSDOS) /* MSDOS already declares this in externs.h */
unsigned sleep();
# endif
#endif

#ifdef ultrix
void exit();
void sleep();
#endif

#if !defined(MAC) && !defined(MSDOS) && !defined(ATARI_ST) && !defined(VMS)
#ifndef AMIGA
#ifdef USG
#if defined(__linux__) || defined(__CYGWIN__)
static struct termios save_termio;
#else
static struct termio save_termio;
#endif
#else
static struct ltchars save_special_chars;
static struct sgttyb save_ttyb;
static struct tchars save_tchars;
static int save_local_chars;
#endif
#endif
#endif

#ifndef MAC
static int curses_on = FALSE;
static WINDOW *savescr;		/* Spare window for saving the screen. -CJS-*/
#ifdef VMS
static WINDOW *tempscr;		/* Spare window for VMS CTRL('R'). */
#endif
#endif

#ifdef MAC
/* Attributes of normal and hilighted characters */
#define ATTR_NORMAL	attrNormal
#define ATTR_HILITED	attrReversed
#endif

#ifndef MAC
#ifdef SIGTSTP
/* suspend()							   -CJS-
   Handle the stop and start signals. This ensures that the log
   is up to date, and that the terminal is fully reset and
   restored.  */
int suspend()
{
#ifdef USG
  /* for USG systems with BSDisms that have SIGTSTP defined, but don't
     actually implement it */
#else
  struct sgttyb tbuf;
  struct ltchars lcbuf;
  struct tchars cbuf;
  int lbuf;
  long time();

  py.misc.male |= 2;
  (void) ioctl(0, TIOCGETP, (char *)&tbuf);
  (void) ioctl(0, TIOCGETC, (char *)&cbuf);
  (void) ioctl(0, TIOCGLTC, (char *)&lcbuf);
#if !defined(atarist) && !defined(__GNUC__)
  (void) ioctl(0, TIOCLGET, (char *)&lbuf);
#endif
  restore_term();
  (void) kill(0, SIGSTOP);
  curses_on = TRUE;
  (void) ioctl(0, TIOCSETP, (char *)&tbuf);
  (void) ioctl(0, TIOCSETC, (char *)&cbuf);
  (void) ioctl(0, TIOCSLTC, (char *)&lcbuf);
#if !defined(atarist) && !defined(__GNUC__)
  (void) ioctl(0, TIOCLSET, (char *)&lbuf);
#endif
  (void) wrefresh(curscr);
  py.misc.male &= ~2;
#endif
  return 0;
}
#endif
#endif

/* initializes curses routines */
void init_curses()
#ifdef MAC
{
  /* Primary initialization is done in mac.c since game is restartable */
  /* Only need to clear the screen here */
  Rect scrn;

  scrn.left = scrn.top = 0;
  scrn.right = SCRN_COLS;
  scrn.bottom = SCRN_ROWS;
  EraseScreen(&scrn);
  UpdateScreen();
}
#else
{
#if 0
  int i, y, x;
#endif

#ifdef AMIGA
  if (opentimer() == 0)
    {
      (void) printf ("Could not open timer device.\n");
      exit (1);
    }
#endif

#ifndef USG
  (void) ioctl(0, TIOCGLTC, (char *)&save_special_chars);
  (void) ioctl(0, TIOCGETP, (char *)&save_ttyb);
  (void) ioctl(0, TIOCGETC, (char *)&save_tchars);
#if !defined(atarist) && !defined(__GNUC__)
  (void) ioctl(0, TIOCLGET, (char *)&save_local_chars);
#endif
#else
#if !defined(VMS) && !defined(MSDOS) && !defined(ATARI_ST)
#ifndef AMIGA
  (void) ioctl(0, TCGETA, (char *)&save_termio);
#endif
#endif
#endif

  /* PC curses returns ERR */
#if defined(USG) && !defined(PC_CURSES) && !defined(AMIGA)
  if (initscr() == NULL)
#else
  if (initscr() == ERR)
#endif
    {
      (void) printf("Error allocating screen in curses package.\n");
      exit(1);
    }
#if defined(_MSC_VER) || defined(__MINGW32__)
  resize_term(24, 80);
  curs_set(1);
#endif
  if (LINES < 24 || COLS < 80)	 /* Check we have enough screen. -CJS- */
    {
      (void) printf("Screen too small for moria (need 80x24, got %lux%lu).\n", COLS, LINES);
      exit (1);
    }
#ifdef SIGTSTP
#if defined(atarist) && defined(__GNUC__)
  (void) signal (SIGTSTP, (__Sigfunc)suspend);
#else
#ifdef  __386BSD__
  (void) signal (SIGTSTP, (sig_t)suspend);
#else
#ifdef DEBIAN_LINUX
  /* (void) signal (SIGTSTP, (sig_t)suspend); */
  /* RENMOD: libc6 defaults to BSD, this expects SYSV */
  (void) sysv_signal (SIGTSTP, suspend);
#else
  (void) signal (SIGTSTP, suspend);
#endif
#endif
#endif
#endif
  if (((savescr = newwin (0, 0, 0, 0)) == NULL)
#ifdef VMS
      || ((tempscr = newwin (0, 0, 0, 0)) == NULL))
#else
    )
#endif
    {
      (void) printf ("Out of memory in starting up curses.\n");
      exit_game();
    }
  (void) clear();
  (void) refresh();
  moriaterm ();

#if 0
  /* This assumes that the terminal is 80 characters wide, which is not
     guaranteed to be true.  */

  /* check tab settings, exit with error if they are not 8 spaces apart */
  (void) move(0, 0);
  for (i = 1; i < 10; i++)
    {
      (void) addch('\t');
      getyx(stdscr, y, x);
      if (y != 0 || x != i*8)
	break;
    }
  if (i != 10)
    {
      msg_print("Tabs must be set 8 spaces apart.");
      exit_game();
    }
#endif
}
#endif

/* Set up the terminal into a suitable state for moria.	 -CJS- */
void moriaterm()
#ifdef MAC
/* Nothing to do on Mac */
{
}
#else
{
#if !defined(MSDOS) && !defined(ATARI_ST) && !defined(VMS)
#ifndef AMIGA
#ifdef USG
#if defined(__linux__) || defined(__CYGWIN__)
  struct termios tbuf;
#else
  struct termio tbuf;
#endif
#else
  struct ltchars lbuf;
  struct tchars buf;
#endif
#endif
#endif

  curses_on = TRUE;
#ifndef BSD4_3
  use_value crmode();
#elif defined(VMS)
  use_value vms_crmode ();
#else
  use_value cbreak();
#endif
  use_value noecho();
  /* can not use nonl(), because some curses do not handle it correctly */
#ifdef MSDOS
  msdos_raw();
#elif defined(AMIGA)
  init_color (0,   0,   0,   0);	/* pen 0 - black */
  init_color (1,1000,1000,1000);	/* pen 1 - white */
  init_color (2,   0, 300, 700);	/* pen 2 - blue */
  init_color (3,1000, 500,   0);	/* pen 3 - orange */
#elif !defined(ATARI_ST) && !defined(VMS)
#ifdef USG
  (void) ioctl(0, TCGETA, (char *)&tbuf);
  /* disable all of the normal special control characters */
  tbuf.c_cc[VINTR] = (char)3; /* control-C */
  tbuf.c_cc[VQUIT] = (char)-1;
  tbuf.c_cc[VERASE] = (char)-1;
  tbuf.c_cc[VKILL] = (char)-1;
  tbuf.c_cc[VEOF] = (char)-1;

  /* don't know what these are for */
  tbuf.c_cc[VEOL] = (char)-1;
#ifdef VEOL2
  tbuf.c_cc[VEOL2] = (char)-1;
#endif

  /* stuff needed when !icanon, i.e. cbreak/raw mode */
  tbuf.c_cc[VMIN] = 1;  /* Input should wait for at least 1 char */
  tbuf.c_cc[VTIME] = 0; /* no matter how long that takes. */

  (void) ioctl(0, TCSETA, (char *)&tbuf);
#else
  /* disable all of the special characters except the suspend char, interrupt
     char, and the control flow start/stop characters */
  (void) ioctl(0, TIOCGLTC, (char *)&lbuf);
  lbuf.t_suspc = (char)26; /* control-Z */
  lbuf.t_dsuspc = (char)-1;
  lbuf.t_rprntc = (char)-1;
  lbuf.t_flushc = (char)-1;
  lbuf.t_werasc = (char)-1;
  lbuf.t_lnextc = (char)-1;
  (void) ioctl(0, TIOCSLTC, (char *)&lbuf);

  (void) ioctl (0, TIOCGETC, (char *)&buf);
  buf.t_intrc = (char)3; /* control-C */
  buf.t_quitc = (char)-1;
  buf.t_startc = (char)17; /* control-Q */
  buf.t_stopc = (char)19; /* control-S */
  buf.t_eofc = (char)-1;
  buf.t_brkc = (char)-1;
  (void) ioctl(0, TIOCSETC, (char *)&buf);
#endif
#endif

#ifdef ATARIST_TC
  raw ();
#endif
}
#endif


/* Dump IO to buffer					-RAK-	*/
void put_buffer(out_str, row, col)
char *out_str;
int row, col;
#ifdef MAC
{
  /* The screen manager handles writes past the edge ok */
  DSetScreenCursor(col, row);
  DWriteScreenStringAttr(out_str, ATTR_NORMAL);
}
#else
{
  vtype tmp_str;

  /* truncate the string, to make sure that it won't go past right edge of
     screen */
  if (col > 79)
    col = 79;
  (void) strncpy (tmp_str, out_str, 79 - col);
  tmp_str [79 - col] = '\0';

  if (mvaddstr(row, col, tmp_str) == ERR)
    {
      abort();
      /* clear msg_flag to avoid problems with unflushed messages */
      msg_flag = 0;
      (void) sprintf(tmp_str, "error in put_buffer, row = %d col = %d\n",
                     row, col);
      prt(tmp_str, 0, 0);
      bell();
      /* wait so user can see error */
      (void) sleep(2);
    }
}
#endif


/* Dump the IO buffer to terminal			-RAK-	*/
void put_qio()
{
  screen_change = TRUE;	   /* Let inven_command know something has changed. */
#ifdef MAC
  UpdateScreen();
#else
  (void) refresh();
#endif
}

/* Put the terminal in the original mode.			   -CJS- */
void restore_term()
{
#ifndef MAC /* Nothing to do on Mac */
#ifdef AMIGA
  closetimer ();
#endif

  if (!curses_on)
    return;
  put_qio();  /* Dump any remaining buffer */
#ifdef MSDOS
  (void) sleep(2);   /* And let it be read. */
#endif
#ifdef VMS
  clear_screen();
  pause_line(15);
#endif
  /* this moves curses to bottom right corner */
#ifdef __CYGWIN__
  /* https://www.cygwin.com/ml/cygwin/2010-05/msg00500.html */
  mvcur(getcury(stdscr), getcurx(stdscr), LINES-1, 0);
#else
  mvcur(stdscr->_cury, stdscr->_curx, LINES-1, 0);
#endif
  endwin();  /* exit curses */
  (void) fflush (stdout);
#ifdef MSDOS
  msdos_noraw();
#endif
  /* restore the saved values of the special chars */
#ifdef USG
#if !defined(MSDOS) && !defined(ATARI_ST) && !defined(VMS) && !defined(AMIGA)
  (void) ioctl(0, TCSETA, (char *)&save_termio);
#endif
#else
  (void) ioctl(0, TIOCSLTC, (char *)&save_special_chars);
  (void) ioctl(0, TIOCSETP, (char *)&save_ttyb);
  (void) ioctl(0, TIOCSETC, (char *)&save_tchars);
#if !defined(atarist) && !defined(__GNUC__)
  (void) ioctl(0, TIOCLSET, (char *)&save_local_chars);
#endif
#endif
  curses_on = FALSE;
#endif
}


void shell_out()
#if defined(atarist) && defined(__GNUC__)
{ char fail_message[80], arg_list[1], *p;
  int  escape_code;

  save_screen();
  clear_screen();
  use_value nocbreak();         /* Must remember to reset terminal modes   */
  use_value echo();             /* or shell i/o will be quite messed up!   */

  p = (char *)getenv("SHELL");
  if (p != (char *)NULL)
    { put_buffer("Escaping to Shell\n",0,0);
      put_qio();
      arg_list[0]=0;
      escape_code = Pexec(0,p,arg_list,0);   /* Launch the shell.          */

      if (escape_code != 0)
         { sprintf(fail_message,"Pexec() error code = %d\n",escape_code);
           put_buffer(fail_message,0,0);
           put_qio();
	   sleep(5);
	 }
    }
  use_value cbreak();        /* Reset the terminal back to CBREAK/NOECHO   */
  use_value noecho();
  clear_screen();            /* Do not want shell data on screen.          */
  restore_screen();
}

#else

#ifdef MAC
{
  alert_error("This command is not implemented on the Macintosh.");
}
#else
#if defined(AMIGA) || defined(ATARIST_TC)
{
  put_buffer("This command is not implemented.\n", 0, 0);
}
#else
#ifdef VMS /* TPP */
{
  int val, istat;
  char *str;

  save_screen();
  /* clear screen and print 'exit' message */
  clear_screen();
  put_buffer("[Entering subprocess, type 'EOJ' to resume your game.]\n",
	     0, 0);
  put_qio();

  use_value vms_nocrmode();
  use_value echo();
  ignore_signals();

  istat = lib$spawn();
  if (!istat)
    lib$signal (istat);

  restore_signals();
  use_value vms_crmode();
  use_value noecho();
  /* restore the cave to the screen */
  restore_screen();
  put_buffer("Welcome back to UMoria.\n", 0, 0);
  save_screen();
  clear_screen();
  put_qio();
  restore_screen();
  (void) wrefresh(curscr);
}
#else
{
#ifdef USG
#if !defined(MSDOS) && !defined(ATARI_ST) && !defined(AMIGA)
#if defined(__linux__) || defined(__CYGWIN__)
  struct termios tbuf;
#else
  struct termio tbuf;
#endif
#endif
#else
  struct sgttyb tbuf;
  struct ltchars lcbuf;
  struct tchars cbuf;
  int lbuf;
#endif
#ifdef MSDOS
  char	*comspec, key;
#else
#ifdef ATARI_ST
  char comstr[80];
  char *str;
  extern char **environ;
#else
  int val;
  char *str;
#endif
#endif

  save_screen();
  /* clear screen and print 'exit' message */
  clear_screen();
#ifndef ATARI_ST
  put_buffer("[Entering shell, type 'exit' to resume your game.]\n",0,0);
#else
  put_buffer("[Escaping to shell]\n",0,0);
#endif
  put_qio();

#ifdef USG
#if !defined(MSDOS) && !defined(ATARI_ST) && !defined(AMIGA)
  (void) ioctl(0, TCGETA, (char *)&tbuf);
#endif
#else
  (void) ioctl(0, TIOCGETP, (char *)&tbuf);
  (void) ioctl(0, TIOCGETC, (char *)&cbuf);
  (void) ioctl(0, TIOCGLTC, (char *)&lcbuf);
  (void) ioctl(0, TIOCLGET, (char *)&lbuf);
#endif
  /* would call nl() here if could use nl()/nonl(), see moriaterm() */
#ifndef BSD4_3
  use_value nocrmode();
#elif defined(VMS)
  use_value vms_nocrmode ();
#else
  use_value nocbreak();
#endif
#ifdef MSDOS
  msdos_noraw();
#endif
  use_value echo();
  ignore_signals();
#ifdef MSDOS		/*{*/
  if ((comspec = getenv("COMSPEC")) == CNIL
  ||  spawnl(P_WAIT, comspec, comspec, CNIL) < 0) {
    clear_screen();	/* BOSS key if shell failed */
    put_buffer("M:\\> ", 0, 0);
    do {
      key = inkey();
    } while (key != '!');
  }

#else		/* MSDOS }{*/
#ifndef ATARI_ST
  val = fork();
  if (val == 0)
    {
#endif
      default_signals();
#ifdef USG
#if !defined(MSDOS) && !defined(ATARI_ST) && !defined(AMIGA)
      (void) ioctl(0, TCSETA, (char *)&save_termio);
#endif
#else
      (void) ioctl(0, TIOCSLTC, (char *)&save_special_chars);
      (void) ioctl(0, TIOCSETP, (char *)&save_ttyb);
      (void) ioctl(0, TIOCSETC, (char *)&save_tchars);
      (void) ioctl(0, TIOCLSET, (char *)&save_local_chars);
#endif
#ifndef MSDOS
      /* close scoreboard descriptor */
      /* it is not open on MSDOS machines */
      (void) fclose(highscore_fp);
#endif
      if (str = getenv("SHELL"))
#ifndef ATARI_ST
	(void) execl(str, str, (char *) 0);
#else
	system(str);
#endif
      else
#ifndef ATARI_ST
	(void) execl("/bin/sh", "sh", (char *) 0);
#endif
      msg_print("Cannot execute shell.");
#ifndef ATARI_ST
      exit(1);
    }
  if (val == -1)
    {
      msg_print("Fork failed. Try again.");
      return;
    }
#if defined(USG) || defined(__386BSD__)
  (void) wait((int *) 0);
#else
  (void) wait((union wait *) 0);
#endif
#endif /* ATARI_ST */
#endif		 /* MSDOS }*/
  restore_signals();
  /* restore the cave to the screen */
  restore_screen();
#ifndef BSD4_3
  use_value crmode();
#elif defined(VMS)
  use_value vms_crmode ();
#else
  use_value cbreak();
#endif
  use_value noecho();
  /* would call nonl() here if could use nl()/nonl(), see moriaterm() */
#ifdef MSDOS
  msdos_raw();
#endif
  /* disable all of the local special characters except the suspend char */
  /* have to disable ^Y for tunneling */
#ifdef USG
#if !defined(MSDOS) && !defined(ATARI_ST)
  (void) ioctl(0, TCSETA, (char *)&tbuf);
#endif
#else
  (void) ioctl(0, TIOCSLTC, (char *)&lcbuf);
  (void) ioctl(0, TIOCSETP, (char *)&tbuf);
  (void) ioctl(0, TIOCSETC, (char *)&cbuf);
  (void) ioctl(0, TIOCLSET, (char *)&lbuf);
#endif
  (void) wrefresh(curscr);
}
#endif
#endif
#endif
#endif

/* Returns a single character input from the terminal.	This silently -CJS-
   consumes ^R to redraw the screen and reset the terminal, so that this
   operation can always be performed at any input prompt.  inkey() never
   returns ^R.	*/
char inkey()
#ifdef MAC
/* The Mac does not need ^R, so it just consumes it */
/* This routine does nothing special with direction keys */
/* Just returns their keypad ascii value (e.g. '0'-'9') */
/* Compare with inkeydir() below */
{
  char ch;
  int dir;
  int shift_flag, ctrl_flag;

  put_qio();
  command_count = 0;

  do {
    macgetkey(&ch, FALSE);
  } while (ch == CTRL('R'));

  dir = extractdir(ch, &shift_flag, &ctrl_flag);
  if (dir != -1)
    ch = '0' + dir;

  return(ch);
}
#else
{
  int i;
#ifdef VMS
  vtype tmp_str;
#endif

  put_qio();			/* Dump IO buffer		*/
  command_count = 0;  /* Just to be safe -CJS- */
  while (TRUE)
    {
#ifdef MSDOS
      i = msdos_getch();
#else
#ifdef VMS
      i = vms_getch ();
#else
      i = getch();
#if defined(atarist) && defined(__GNUC__)
/* for some reason a keypad number produces an initial negative number. */
      if (i<0) i = getch();
#endif
#endif
#endif

#ifdef VMS
      if (i == 27) /* if ESCAPE key, then we probably have a keypad key */
	{
	  i = vms_getch();
	  if (i == 'O') /* Now it is definitely a numeric keypad key */
	    {
	      i = vms_getch();
	      switch (i)
		{
		  case 'p': i = '0'; break;
		  case 'q' : i = '1'; break;
		  case 'r' : i = '2'; break;
		  case 's' : i = '3'; break;
		  case 't' : i = '4'; break;
		  case 'u' : i = '5'; break;
		  case 'v' : i = '6'; break;
		  case 'w' : i = '7'; break;
		  case 'x' : i = '8'; break;
		  case 'y' : i = '9'; break;
		  case 'm' : i = '-'; break;
		  case 'M' : i = 10; break; /* Enter = RETURN */
		  case 'n' : i = '.'; break;
		  default : while (kbhit()) (void) vms_getch();
		  }
	    }
	  else
	    {
	      while (kbhit())
		(void) vms_getch();
	    }
	}
#endif /* VMS */

      /* some machines may not sign extend. */
      if (i == EOF)
	{
	  eof_flag++;
	  /* avoid infinite loops while trying to call inkey() for a -more-
	     prompt. */
	  msg_flag = FALSE;

	  (void) refresh ();
	  if (!character_generated || character_saved)
	    exit_game();
	  disturb(1, 0);
	  if (eof_flag > 100)
	    {
	      /* just in case, to make sure that the process eventually dies */
	      panic_save = 1;
	      (void) strcpy(died_from, "(end of input: panic saved)");
	      if (!save_char())
		{
		  (void) strcpy(died_from, "panic: unexpected eof");
		  death = TRUE;
		}
	      exit_game();
	    }
	  return ESCAPE;
	}
      if (i != CTRL('R'))
	return (char)i;
#ifdef VMS
      /* Refresh does not work right under VMS, so use a brute force. */
      overwrite (stdscr, tempscr);
      clear_screen();
      put_qio();
      overwrite (tempscr, stdscr);
      touchwin (stdscr);
      (void) wrefresh (stdscr);
#endif
      (void) wrefresh (curscr);
      moriaterm();
    }
}
#endif


#ifdef MAC
char inkeydir()
/* The Mac does not need ^R, so it just consumes it */
/* This routine translates the direction keys in rogue-like mode */
/* Compare with inkeydir() below */
{
  char ch;
  int dir;
  int shift_flag, ctrl_flag;
  static char tab[9] = {
	'b',		'j',		'n',
	'h',		'.',		'l',
	'y',		'k',		'u'
  };
  static char shifttab[9] = {
	'B',		'J',		'N',
	'H',		'.',		'L',
	'Y',		'K',		'U'
  };
  static char ctrltab[9] = {
	CTRL('B'),	CTRL('J'),	CTRL('N'),
	CTRL('H'),	'.',		CTRL('L'),
	CTRL('Y'),	CTRL('K'),	CTRL('U')
  };

  put_qio();
  command_count = 0;

  do {
    macgetkey(&ch, FALSE);
  } while (ch == CTRL('R'));

  dir = extractdir(ch, &shift_flag, &ctrl_flag);

  if (dir != -1) {
    if (!rogue_like_commands) {
      ch = '0' + dir;
    }
    else {
      if (ctrl_flag)
	ch = ctrltab[dir - 1];
      else if (shift_flag)
	ch = shifttab[dir - 1];
      else
	ch = tab[dir - 1];
    }
  }

  return(ch);
}
#endif


/* Flush the buffer					-RAK-	*/
void flush()
{
#ifdef MAC
/* Removed put_qio() call.  Reduces flashing.  Doesn't seem to hurt. */
  FlushScreenKeys();
#elif defined(_MSC_VER) || defined(__MINGW32__)
  flushinp();
#elif defined(MSDOS)
  while (kbhit())
	(void) getch();
#elif defined(VMS)
  while (kbhit ())
    (void) vms_getch();
#else
  /* the code originally used ioctls, TIOCDRAIN, or TIOCGETP/TIOCSETP, or
     TCGETA/TCSETAF, however this occasionally resulted in loss of output,
     the happened especially often when rlogin from BSD to SYS_V machine,
     using check_input makes the desired effect a bit clearer */
  /* wierd things happen on EOF, don't try to flush input in that case */
  if (!eof_flag)
    while (check_input(0));
#endif

  /* used to call put_qio() here to drain output, but it is not necessary */
}


/* Clears given line of text				-RAK-	*/
void erase_line(row, col)
int row;
int col;
#ifdef MAC
{
  Rect line;

  if (row == MSG_LINE && msg_flag)
    msg_print(CNIL);

  line.left = col;
  line.top = row;
  line.right = SCRN_COLS;
  line.bottom = row + 1;
  DEraseScreen(&line);
}
#else
{
  if (row == MSG_LINE && msg_flag)
    msg_print(CNIL);
  (void) move(row, col);
  clrtoeol();
}
#endif


/* Clears screen */
void clear_screen()
#ifdef MAC
{
  Rect area;

  if (msg_flag)
    msg_print(CNIL);

  area.left = area.top = 0;
  area.right = SCRN_COLS;
  area.bottom = SCRN_ROWS;
  DEraseScreen(&area);
}
#else
{
  if (msg_flag)
    msg_print(CNIL);
#ifdef VMS
  /* Clear doesn't work right under VMS, so use brute force. */
  (void) clearok (stdscr, TRUE);
  (void) wclear(stdscr);
  (void) clearok (stdscr, FALSE);
#else
  (void) clear();
#endif
}
#endif

void clear_from (row)
int row;
#ifdef MAC
{
  Rect area;

  area.left = 0;
  area.top = row;
  area.right = SCRN_COLS;
  area.bottom = SCRN_ROWS;
  DEraseScreen(&area);
}
#else
{
  (void) move(row, 0);
  clrtobot();
}
#endif


/* Outputs a char to a given interpolated y, x position	-RAK-	*/
/* sign bit of a character used to indicate standout mode. -CJS */
void print(ch, row, col)
#if defined(_MSC_VER) || defined(__MINGW32__)
/* Use int to avoid corrupting high cp437 values */
int ch;
#else
char ch;
#endif
int row;
int col;
#ifdef MAC
{
  char cnow, anow;

  row -= panel_row_prt;/* Real co-ords convert to screen positions */
  col -= panel_col_prt;

  GetScreenCharAttr(&cnow, &anow, col, row);	/* Check current */

  /* If char is already set, ignore op */
  if ((cnow != ch) || (anow != ATTR_NORMAL))
    DSetScreenCharAttr(ch & 0x7F,
		       (ch & 0x80) ? attrReversed : attrNormal,
		       col, row);
}
#else
{
  vtype tmp_str;
#ifdef PDC_WIDE
  /* translate CP437 to UTF equivalents when using PDCurses in wide character mode */
  const cchar_t wch = ToUTF(ch);
#endif

  row -= panel_row_prt;/* Real co-ords convert to screen positions */
  col -= panel_col_prt;
#if defined(PDC_WIDE)
  if (mvadd_wch(row, col, &wch) == ERR)
#else
  if (mvaddch (row, col, ch) == ERR)
#endif
  {
    abort();
    /* clear msg_flag to avoid problems with unflushed messages */
    msg_flag = 0;
    (void) sprintf(tmp_str, "error in print, row = %d col = %d\n",
                   row, col);
    prt(tmp_str, 0, 0);
    bell();
    /* wait so user can see error */
    (void) sleep(2);
  }
}
#endif


/* Moves the cursor to a given interpolated y, x position	-RAK-	*/
void move_cursor_relative(row, col)
int row;
int col;
#ifdef MAC
{
  row -= panel_row_prt;/* Real co-ords convert to screen positions */
  col -= panel_col_prt;

  DSetScreenCursor(col, row);
}
#else
{
  vtype tmp_str;

  row -= panel_row_prt;/* Real co-ords convert to screen positions */
  col -= panel_col_prt;
  if (move (row, col) == ERR)
    {
      abort();
      /* clear msg_flag to avoid problems with unflushed messages */
      msg_flag = 0;
      (void) sprintf(tmp_str,
		     "error in move_cursor_relative, row = %d col = %d\n",
		     row, col);
      prt(tmp_str, 0, 0);
      bell();
      /* wait so user can see error */
      (void) sleep(2);
    }
}
#endif


/* Print a message so as not to interrupt a counted command. -CJS- */
void count_msg_print(p)
char *p;
{
  int i;

  i = command_count;
  msg_print(p);
  command_count = i;
}


/* Outputs a line to a given y, x position		-RAK-	*/
void prt(str_buff, row, col)
char *str_buff;
int row;
int col;
#ifdef MAC
{
  Rect line;

  if (row == MSG_LINE && msg_flag)
    msg_print(CNIL);

  line.left = col;
  line.top = row;
  line.right = SCRN_COLS;
  line.bottom = row + 1;
  DEraseScreen(&line);

  put_buffer(str_buff, row, col);
}
#else
{
  if (row == MSG_LINE && msg_flag)
    msg_print(CNIL);
  (void) move(row, col);
  clrtoeol();
  put_buffer(str_buff, row, col);
}
#endif


/* move cursor to a given y, x position */
void move_cursor(row, col)
int row, col;
#ifdef MAC
{
  DSetScreenCursor(col, row);
}
#else
{
  (void) move (row, col);
}
#endif


/* Outputs message to top line of screen				*/
/* These messages are kept for later reference.	 */
void msg_print(str_buff)
char *str_buff;
{
  register int old_len, new_len;
  int combine_messages = FALSE;
  char in_char;
#ifdef MAC
  Rect line;
#endif

  if (msg_flag)
    {
      old_len = strlen(old_msg[last_msg]) + 1;

      /* If the new message and the old message are short enough, we want
	 display them together on the same line.  So we don't flush the old
	 message in this case.  */

      if (str_buff)
	new_len = strlen (str_buff);
      else
	new_len = 0;

      if (! str_buff || (new_len + old_len + 2 >= 73))
	{
	  /* ensure that the complete -more- message is visible. */
	  if (old_len > 73)
	    old_len = 73;
	  put_buffer(" -more-", MSG_LINE, old_len);
	  /* let sigint handler know that we are waiting for a space */
	  wait_for_more = 1;
	  do
	    {
	      in_char = inkey();
	    }
	  while ((in_char != ' ') && (in_char != ESCAPE) && (in_char != '\n')
		 && (in_char != '\r'));
	  wait_for_more = 0;
	}
      else
	combine_messages = TRUE;
    }

  if (! combine_messages)
    {
#ifdef MAC
      line.left = 0;
      line.top = MSG_LINE;
      line.right = SCRN_COLS;
      line.bottom = MSG_LINE+1;
      DEraseScreen(&line);
#else
      (void) move(MSG_LINE, 0);
      clrtoeol();
#endif
    }

  /* Make the null string a special case.  -CJS- */
  if (str_buff)
    {
      command_count = 0;
      msg_flag = TRUE;

      /* If the new message and the old message are short enough, display
	 them on the same line.  */

      if (combine_messages)
	{
	  put_buffer (str_buff, MSG_LINE, old_len + 2);
	  strcat (old_msg[last_msg], "  ");
	  strcat (old_msg[last_msg], str_buff);
	}
      else
	{
	  put_buffer(str_buff, MSG_LINE, 0);
	  last_msg++;
	  if (last_msg >= MAX_SAVE_MSG)
	    last_msg = 0;
	  (void) strncpy(old_msg[last_msg], str_buff, VTYPESIZ);
	  old_msg[last_msg][VTYPESIZ - 1] = '\0';
	}
    }
  else
    msg_flag = FALSE;
}


/* Used to verify a choice - user gets the chance to abort choice.  -CJS- */
int get_check(prompt)
char *prompt;
{
  int res;
#ifdef MAC
  long y, x;		/* ??? Should change to int or short.  */
#else
  int y, x;
#endif

  prt(prompt, 0, 0);
#ifdef MAC
  GetScreenCursor(&x, &y);
#else
  getyx(stdscr, y, x);
#if defined(lint)
  /* prevent message 'warning: y is unused' */
  x = y;
#endif
#ifdef LINT_ARGS
  /* prevent message about y never used for MSDOS systems */
  res = y;
#endif
#endif

  if (x > 73)
    (void) move(0, 73);
#ifdef MAC
  DWriteScreenStringAttr(" [y/n]", ATTR_NORMAL);
#else
  (void) addstr(" [y/n]");
#endif
  do
    {
      res = inkey();
    }
  while(res == ' ');
  erase_line(0, 0);
  if (res == 'Y' || res == 'y')
    return TRUE;
  else
    return FALSE;
}

/* Prompts (optional) and returns ord value of input char	*/
/* Function returns false if <ESCAPE> is input	*/
int get_com(prompt, command)
char *prompt;
char *command;
{
  int res;

  if (prompt)
    prt(prompt, 0, 0);
  *command = inkey();
  if (*command == ESCAPE)
    res = FALSE;
  else
    res = TRUE;
  erase_line(MSG_LINE, 0);
  return(res);
}

#ifdef MAC
/* Same as get_com(), but translates direction keys from keypad */
int get_comdir(prompt, command)
char *prompt;
char *command;
{
  int res;

  if (prompt)
    prt(prompt, 0, 0);
  *command = inkeydir();
  if (*command == ESCAPE)
    res = FALSE;
  else
    res = TRUE;
  erase_line(MSG_LINE, 0);
  return(res);
}
#endif


/* Gets a string terminated by <RETURN>		*/
/* Function returns false if <ESCAPE> is input	*/
int get_string(in_str, row, column, slen)
char *in_str;
int row, column, slen;
{
  register int start_col, end_col, i;
  char *p;
  int flag, aborted;
#ifdef MAC
  Rect area;
#endif
#ifdef PDC_WIDE
  /* translate CP437 to UTF equivalents when using PDCurses in wide character mode */
  cchar_t wch;
#endif

  aborted = FALSE;
  flag	= FALSE;
#ifdef MAC
  area.left = column;
  area.top = row;
  area.right = column + slen;
  area.bottom = row + 1;
  DEraseScreen(&area);
  DSetScreenCursor(column, row);
#else
  (void) move(row, column);
  for (i = slen; i > 0; i--)
#if defined(PDC_WIDE)
    add_wch(&cp437utf[' ']);
#else
    (void) addch(' ');
#endif
  (void) move(row, column);
#endif
  start_col = column;
  end_col = column + slen - 1;
  if (end_col > 79)
    {
      slen = 80 - column;
      end_col = 79;
    }
  p = in_str;
  do
  {
    i = inkey();
    switch(i)
    {
    case ESCAPE:
      aborted = TRUE;
      break;
    case CTRL('J'): case CTRL('M'):
      flag = TRUE;
      break;
    case MORIA_DELETE: case CTRL('H'):
      if (column > start_col)
      {
        column--;
        put_buffer(" ", row, column);
        move_cursor(row, column);
        *--p = '\0';
      }
      break;
    default:
      if (!isprint(i) || column > end_col)
        bell();
      else
      {
#ifdef MAC
        DSetScreenCursor(column, row);
        DWriteScreenCharAttr((char) i, ATTR_NORMAL);
#elif defined(PDC_WIDE) /* convert UTF to CP437 equivalents in PDCurses wide character mode */
        wch = ToUTF(i);
        mvadd_wch(row, column, &wch);
#else
        use_value2 mvaddch(row, column, (char)i);
#endif
        *p++ = i;
        column++;
      }
      break;
    }
  } while (!flag && !aborted);
  if (aborted)
    return(FALSE);
  /* Remove trailing blanks	*/
  while (p > in_str && p[-1] == ' ')
    p--;
  *p = '\0';
  return(TRUE);
}


/* Pauses for user response before returning		-RAK-	*/
void pause_line(prt_line)
int prt_line;
{
  prt("[Press any key to continue.]", prt_line, 23);
  (void) inkey();
  erase_line(prt_line, 0);
}


/* Pauses for user response before returning		-RAK-	*/
/* NOTE: Delay is for players trying to roll up "perfect"	*/
/*	characters.  Make them wait a bit.			*/
void pause_exit(prt_line, delay)
int prt_line;
int delay;
{
  char dummy;

  prt("[Press any key to continue, or Q to exit.]", prt_line, 10);
  dummy = inkey();
  if (dummy == 'Q')
    {
      erase_line(prt_line, 0);
#ifndef MSDOS		/* PCs are slow enough as is  -dgk */
      if (delay > 0)  (void) sleep((unsigned)delay);
#else
      /* prevent message about delay unused */
      dummy = delay;
#endif
#ifdef MAC
      enablefilemenu(FALSE);
      exit_game();
      enablefilemenu(TRUE);
#else
      exit_game();
#endif
    }
  erase_line(prt_line, 0);
}

#ifdef MAC
void save_screen()
{
  mac_save_screen();
}

void restore_screen()
{
  mac_restore_screen();
}
#else
void save_screen()
{
  overwrite(stdscr, savescr);
}

void restore_screen()
{
  overwrite(savescr, stdscr);
  touchwin(stdscr);
}
#endif

void bell()
{
  put_qio();

  /* The player can turn off beeps if he/she finds them annoying.  */
  if (! sound_beep_flag)
    return;

#ifdef MAC
  mac_beep();
#elif defined(_MSC_VER) || defined(__MINGW32__)
  beep();
#else
  (void) write(1, "\007", 1);
#endif
}

/* definitions used by screen_map() */
/* index into border character array */
#define TL 0	/* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4	/* horizontal edge */
#define VE 5

/* character set to use */
#ifdef MSDOS
# ifdef ANSI
#   define CH(x)	(ansi ? screen_border[0][x] : screen_border[1][x])
# else
#   define CH(x)	(screen_border[1][x])
# endif
#else
#   define CH(x)	(screen_border[0][x])
#endif

  /* Display highest priority object in the RATIO by RATIO area */
#define	RATIO 3

void screen_map()
{
  register int	i, j;
  static int8u screen_border[2][6] = {
    {'+', '+', '+', '+', '-', '|'},	/* normal chars */
    {201, 187, 200, 188, 205, 186}	/* graphics chars */
  };
  int8u map[MAX_WIDTH / RATIO + 1];
  int8u tmp;
  int priority[256];
  int row, orow, col, myrow, mycol = 0;
#if defined(PDC_WIDE)
  register int k;
  wchar_t wprntscrnbuf[80];
#elif !defined(MAC)
  char prntscrnbuf[80];
#endif

  for (i = 0; i < 256; i++)
    priority[i] = 0;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;
#ifdef MSDOS
  priority[wallsym] = -5;
  priority[floorsym] = -10;
#else
#ifndef ATARI_ST
  priority['#'] = -5;
#else
  priority[(unsigned char)240] = -5;
#endif
  priority['.'] = -10;
#endif
  priority['\''] = -3;
  priority[' '] = -15;

  save_screen();
  clear_screen();
#ifdef MAC
  DSetScreenCursor(0, 0);
  DWriteScreenCharAttr(CH(TL), ATTR_NORMAL);
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
    DWriteScreenCharAttr(CH(HE), ATTR_NORMAL);
  DWriteScreenCharAttr(CH(TR), ATTR_NORMAL);
#else
# if defined(PDC_WIDE)
  mvadd_wch(0, 0, &cp437utf[CH(TL)]);
# else
  use_value2 mvaddch(0, 0, CH(TL));
# endif
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
# if defined(PDC_WIDE)
    add_wch(&cp437utf[CH(HE)]);
  add_wch(&cp437utf[CH(TR)]);
# else
    (void) addch(CH(HE));
  (void) addch(CH(TR));
# endif
#endif
  orow = -1;
  map[MAX_WIDTH / RATIO] = '\0';
  for (i = 0; i < MAX_HEIGHT; i++)
  {
    row = i / RATIO;
    if (row != orow)
    {
      if (orow >= 0)
      {
#ifdef MAC
        DSetScreenCursor(0, orow+1);
        DWriteScreenCharAttr(CH(VE), ATTR_NORMAL);
        DWriteScreenString((char *) map);
        DWriteScreenCharAttr(CH(VE), ATTR_NORMAL);
#elif defined(PDC_WIDE)
        wprntscrnbuf[0] = cp437utf[CH(VE)];
        for (k = 0; k < MAX_WIDTH / RATIO; ++k)
          wprntscrnbuf[k + 1] = ToUTF(map[k]);
        wprntscrnbuf[MAX_WIDTH / RATIO + 1] = cp437utf[CH(VE)];
        wprntscrnbuf[MAX_WIDTH / RATIO + 2] = '\0';
        mvaddwstr(orow + 1, 0, wprntscrnbuf);
#else
     /* can not use mvprintw() on ibmpc, because PC-Curses is horribly
        written, and mvprintw() causes the fp emulation library to be
        linked with PC-Moria, makes the program 10K bigger */
        (void) sprintf(prntscrnbuf,"%c%s%c",CH(VE), map, CH(VE));
        use_value2 mvaddstr(orow + 1, 0, prntscrnbuf);
#endif
      }
      for (j = 0; j < MAX_WIDTH / RATIO; j++)
        map[j] = ' ';
      orow = row;
    }
    for (j = 0; j < MAX_WIDTH; j++)
    {
      col = j / RATIO;
      tmp = loc_symbol(i, j);
      if (priority[map[col]] < priority[tmp])
        map[col] = tmp;
      if (map[col] == '@')
      {
        mycol = col + 1; /* account for border */
        myrow = row + 1;
      }
    }
  }
  if (orow >= 0)
  {
#ifdef MAC
    DSetScreenCursor(0, orow+1);
    DWriteScreenCharAttr(CH(VE), ATTR_NORMAL);
    DWriteScreenString((char *) map);
    DWriteScreenCharAttr(CH(VE), ATTR_NORMAL);
#elif defined(PDC_WIDE)
    wprntscrnbuf[0] = cp437utf[CH(VE)];
    for (k = 0; k < MAX_WIDTH / RATIO; ++k)
      wprntscrnbuf[k+1] = ToUTF(map[k]);
    wprntscrnbuf[MAX_WIDTH / RATIO + 1] = cp437utf[CH(VE)];
    wprntscrnbuf[MAX_WIDTH / RATIO + 2] = '\0';
    mvaddwstr(orow + 1, 0, wprntscrnbuf);
#else
    (void) sprintf(prntscrnbuf,"%c%s%c",CH(VE), map, CH(VE));
    use_value2 mvaddstr(orow + 1, 0, prntscrnbuf);
#endif
  }
#ifdef MAC
  DSetScreenCursor(0, orow + 2);
  DWriteScreenCharAttr(CH(BL), ATTR_NORMAL);
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
    DWriteScreenCharAttr(CH(HE), ATTR_NORMAL);
  DWriteScreenCharAttr(CH(BR), ATTR_NORMAL);
#else
#if defined(PDC_WIDE)
  mvadd_wch(orow + 2, 0, &cp437utf[CH(BL)]);
#else
  use_value2 mvaddch(orow + 2, 0, CH(BL));
#endif
  for (i = 0; i < MAX_WIDTH / RATIO; i++)
#if defined(PDC_WIDE)
    add_wch(&cp437utf[CH(HE)]);
  add_wch(&cp437utf[CH(BR)]);
#else
    (void) addch(CH(HE));
  (void) addch(CH(BR));
#endif
#endif

#ifdef MAC
  DSetScreenCursor(23, 23);
  DWriteScreenStringAttr("Hit any key to continue", ATTR_NORMAL);
  if (mycol > 0)
    DSetScreenCursor(mycol, myrow);
#else
  use_value2 mvaddstr(23, 23, "Hit any key to continue");
  if (mycol > 0)
    (void) move(myrow, mycol);
#endif
  (void) inkey();
  restore_screen();
}
