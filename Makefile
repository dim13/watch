#	$Id$

PROG=	watch
BINDIR=	/usr/local/bin
MAN=	watch.1
#CFLAGS+=-ggdb
PDADD= ${LIBCURSES}
LDADD= -lcurses

. include <bsd.prog.mk>
