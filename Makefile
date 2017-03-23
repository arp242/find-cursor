all:
	cc find-cursor.c -o find-cursor -lX11 -lXext -lXfixes

install:
	install --strip -o root -g root find-cursor /bin/
