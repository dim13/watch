PROG=	watch
BINDIR=	/usr/local/bin
NOMAN=	noman
PDADD= ${LIBCURSES}
LDADD= -lcurses

. include <bsd.prog.mk>