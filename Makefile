CC?=cc
CFLAGS?=-std=c99 -pedantic -Wall -Os

all:
	${CC} ${CFLAGS} -o find-cursor find-cursor.c -lX11 -lXext -lXfixes

install:
	install --strip -o root -g root find-cursor /bin/
