CC?=cc

all:
	${CC} -o find-cursor find-cursor.c -lX11 -lXext -lXfixes

install:
	install --strip -o root -g root find-cursor /bin/
