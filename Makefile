all:
	cc find-cursor.c -o find-cursor -lX11

install:
	install --strip -o root -g root find-cursor /bin/
