PROG	 = watch
LIBS	 = ncurses
CFLAGS	+= $(shell pkg-config --cflags $(LIBS))
LDLIBS	+= $(shell pkg-config --libs   $(LIBS))

all: ${PROG}

clean:
	${RM} ${PROG} *.o
