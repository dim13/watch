#	$Id$

PROG=	watch
BINDIR=	/usr/local/bin
NOMAN=	watch.1
PDADD= ${LIBCURSES}
LDADD= -lcurses

. include <bsd.prog.mk>