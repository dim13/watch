#	$Id$

PROG=		watch
BINDIR=		/usr/local/bin
MAN=		watch.1
DPADD=		${LIBCURSES}
LDADD=		-lcurses

. include <bsd.prog.mk>
