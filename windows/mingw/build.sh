PREFIX=i686-w64-mingw32-
CC=${PREFIX}gcc
STRIP=${PREFIX}strip
CCOPTS="-D_MSC_VER -DPDC_WIDE -Wall"
PDCROOT=../../../../../pdcurses/git/PDCurses
PDCEXT=${PDCROOT}/win32
PDCINC="-I${PDCROOT} -I${PDCEXT}"
PDCLIB=${PDCEXT}/pdcurses.a

ls ../../source/*.c | xargs -n 1 basename -s \.c | xargs -t -n 1 -i ${CC} ${CCOPTS} -I../../source -I.. ${PDCINC} -c ../../source/{}.c -o ./{}.o

echo ${CC} ${CCOPTS} -I../../source -I.. ${PDCINC} -c ../ms_misc.c -o ./ms_misc.o
${CC} ${CCOPTS} -I../../source -I.. ${PDCINC} -c ../ms_misc.c -o ./ms_misc.o

echo ${CC} ${CCOPTS} *.o ${PDCLIB} -o moria.exe
${CC} ${CCOPTS} *.o ${PDCLIB} -o moria.exe

echo ${STRIP} moria.exe
${STRIP} moria.exe
