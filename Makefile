CC      ?= cc
PREFIX  ?= /usr/local
NAME    ?= find-cursor

CFLAGS  += -std=c99 -pedantic -Wall -Wextra -Wpedantic -Os
LDFLAGS += -L/usr/lib
LIBS += -lX11 -lXext -lXfixes

.PHONY: all clean install uninstall

all:
	${CC} ${CFLAGS} ${LDFLAGS} -o ${NAME} *.c ${LIBS}

clean:
	rm -f ${NAME}

install:
	install -Dm755 ${NAME} ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
