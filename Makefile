#	$Id$

PROG=		watch
BINDIR=		/usr/local/bin
MAN=		watch.1
DPADD=		${LIBCURSES}
CFLAGS+=	-W -Wall
LDADD=		-lcurses

. include <bsd.prog.mk>
