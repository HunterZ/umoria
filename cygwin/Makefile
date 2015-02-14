# DISTDIR is the directory where the moria binary while be put
# DISTDIR is also where the other files (score, news, hours) will be put
# DISTDIR must be the same directory defined in config.h
# OWNER is who you want the game to be chown to.
# GROUP is who you wnat the game to be chgrp to.
SRCROOT = $(shell pwd)/..
DISTDIR = $(SRCROOT)/cygwin/dist
#OWNER = $(USER)
#GROUP = None

# For testing and debugging the program, it is best to use this line.
# CFLAGS = -g
# For playing the game, you may want to use this line
CFLAGS = -O3 -g -I../source

# For BSD Systems
# CURSES = -lcurses -ltermcap
# For SYS V Systems
CURSES = -lcurses

# Normal systems don't require anything here.
LFLAGS = 

CC = cc

SRCS = main.c misc1.c misc2.c misc3.c misc4.c store1.c files.c io.c \
	create.c desc.c generate.c sets.c dungeon.c creature.c death.c \
	eat.c help.c magic.c potions.c prayer.c save.c staffs.c wands.c \
	scrolls.c spells.c wizard.c store2.c signals.c moria1.c moria2.c \
	moria3.c moria4.c monsters.c treasure.c variable.c rnd.c recall.c \
	$(SRCROOT)/cygwin/unix.c player.c tables.c

OBJS = main.o misc1.o misc2.o misc3.o misc4.o store1.o files.o io.o \
	create.o desc.o generate.o sets.o dungeon.o creature.o death.o \
	eat.o help.o magic.o potions.o prayer.o save.o staffs.o wands.o \
	scrolls.o spells.o wizard.o store2.o signals.o moria1.o moria2.o \
	moria3.o moria4.o monsters.o treasure.o variable.o rnd.o recall.o \
	$(SRCROOT)/cygwin/unix.o player.o tables.o

LIBFILES = COPYING hours news origcmds.hlp owizcmds.hlp roglcmds.hlp \
	rwizcmds.hlp version.hlp welcome.hlp $(SRCROOT)/cygwin/readme.txt

DOCFILES = dragon.inf ERRORS exp.doc faq FEATURES.NEW history moria.6 \
	moria.man moria1.ms moria1.txt moria2.ms moria2.txt \
	moria2.txt.orig pronounc README spells.doc where.inf

moria : $(OBJS)
	$(CC) -o moria $(CFLAGS) $(OBJS) $(CURSES) $(LFLAGS)

lintout : $(SRCS)
	lint $(SRCS) $(CURSES) > lintout

lintout2 : $(SRCS)
	lint -bach $(SRCS) $(CURSES) > lintout

TAGS : $(SRCS)
	ctags -x $(SRCS) > TAGS

# you must define DISTDIR before installing
install:
# create dist directory tree
	mkdir -p $(DISTDIR)/doc
# copy binary
	cp $(SRCROOT)/source/moria.exe $(DISTDIR)
# copy data files and set permissions
	(cd $(SRCROOT)/files; cp $(LIBFILES) $(DISTDIR))
	(cd $(DISTDIR); chmod 444 $(LIBFILES))
	(cd $(DISTDIR); touch scores; chmod 644 scores)
# copy doc files and set permissions
	(cd $(SRCROOT)/doc; cp $(DOCFILES) $(DISTDIR)/doc)
	(cd $(DISTDIR)/doc; chmod 444 $(DOCFILES))
# copy cygwin dependencies
	cp /usr/bin/cygncursesw-10.dll $(DISTDIR)
	cp /usr/bin/cygwin1.dll $(DISTDIR)

# If you are short on disk space, or aren't interested in debugging moria.
	strip $(DISTDIR)/moria.exe

clean:
	rm -f $(SRCROOT)/source/*.o $(SRCROOT)/cygwin/*.o $(SRCROOT)/source/moria.exe
	@echo
	@echo NOTE: any files in $(DISTDIR) will need to be manually deleted

create.o: constant.h types.h externs.h config.h
creature.o: constant.h types.h externs.h config.h
death.o: constant.h types.h externs.h config.h
desc.o: constant.h types.h externs.h config.h
dungeon.o: constant.h types.h externs.h config.h
eat.o: constant.h types.h externs.h config.h
files.o: constant.h types.h externs.h config.h
generate.o: constant.h types.h externs.h config.h
help.o: constant.h types.h externs.h config.h
io.o: constant.h types.h externs.h config.h
magic.o: constant.h types.h externs.h config.h
main.o: constant.h types.h externs.h config.h
misc1.o: constant.h types.h externs.h config.h
misc2.o: constant.h types.h externs.h config.h
misc3.o: constant.h types.h externs.h config.h
misc4.o: constant.h types.h externs.h config.h
monsters.o: constant.h types.h config.h
moria1.o: constant.h types.h externs.h config.h
moria2.o: constant.h types.h externs.h config.h
moria3.o: constant.h types.h externs.h config.h
moria4.o: constant.h types.h externs.h config.h
player.o: constant.h types.h config.h
potions.o: constant.h types.h externs.h config.h
prayer.o: constant.h types.h externs.h config.h
recall.o: constant.h config.h types.h externs.h
rnd.o: constant.h types.h
save.o: constant.h types.h externs.h config.h
scrolls.o: constant.h types.h externs.h config.h
sets.o: constant.h config.h
signals.o: constant.h types.h externs.h config.h
spells.o: constant.h types.h externs.h config.h
staffs.o: constant.h types.h externs.h config.h
store1.o: constant.h types.h externs.h config.h
store2.o: constant.h types.h externs.h config.h
tables.o: constant.h types.h config.h
treasure.o: constant.h types.h config.h
$(SRCROOT)/cygwin/unix.o: constant.h config.h types.h externs.h
variable.o: constant.h types.h config.h
wands.o: constant.h types.h externs.h config.h
wizard.o: constant.h types.h externs.h config.h
