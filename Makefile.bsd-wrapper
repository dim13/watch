PROG=		watch
BINDIR=		/usr/local/bin
MAN=		watch.1
DPADD=		${LIBCURSES}
CFLAGS+=	-W -Wall
LDADD=		-lcurses

README.md:	watch.1
	mandoc -Tmarkdown ${.ALLSRC} > ${.TARGET}

. include <bsd.prog.mk>
