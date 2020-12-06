CC?=cc
CFLAGS?=-std=c99 -pedantic -Wall -Os
PREFIX?=/usr/local

all:
	${CC} ${CFLAGS} ${LDFLAGS} -o find-cursor find-cursor.c -lX11 -lXext -lXfixes

install:
	install -Dm755 find-cursor $(DESTDIR)$(PREFIX)/bin/find-cursor
