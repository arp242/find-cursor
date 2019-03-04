CC?=cc
CFLAGS?=-std=c99 -pedantic -Wall -Os
STRIP?=strip
PREFIX?=/usr

all:
	${CC} ${CFLAGS} ${LDFLAGS} -o find-cursor find-cursor.c -lX11 -lXext -lXfixes

install:
	$(STRIP) find-cursor
	install -Dm755 find-cursor $(DESTDIR)$(PREFIX)/bin/find-cursor
