CC?=cc
CFLAGS?=-std=c99 -pedantic -Wall -Os
PREFIX?=/usr/local
MANPREFIX?=$(PREFIX)/share/man

all:
	${CC} ${CFLAGS} ${LDFLAGS} -o find-cursor find-cursor.c -lX11 -lXext -lXfixes

install:
	install -Dm755 find-cursor $(DESTDIR)$(PREFIX)/bin/find-cursor
	sed "s/VERSION/`git tag | tail -1 | tr -d v`/g" < find-cursor.1 > $(DESTDIR)$(MANPREFIX)/man1/find-cursor.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/find-cursor.1
