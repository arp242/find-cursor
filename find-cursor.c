/*
 * http://code.arp242.net/find-cursor
 * Copyright © 2015 Martin Tournoij <martin@arp242.net>
 * See below for full copyright
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>


// Some variables you can play with :-)
int size = 220;
int step = 40;
int speed = 400;
int line_width = 2;
char color_name[] = "black";


int main(int argc, char* argv[]) {
	// Setup display and such
	char *display_name = getenv("DISPLAY");
	if (!display_name) {
		fprintf(stderr, "%s: cannot connect to X server '%s'\n", argv[0], display_name);
		exit(1);
	}

	Display *display = XOpenDisplay(display_name);
	int screen = DefaultScreen(display);

	// Get the mouse cursor position
	int win_x, win_y, root_x, root_y = 0;
	unsigned int mask = 0;
	Window child_win, root_win;
	XQueryPointer(display, XRootWindow(display, screen),
		&child_win, &root_win,
		&root_x, &root_y, &win_x, &win_y, &mask);

	// Create a window at the mouse position
	XSetWindowAttributes window_attr;
	window_attr.override_redirect = 1;
	Window window = XCreateWindow(display, XRootWindow(display, screen),
		root_x - size/2, root_y - size/2,   // x, y position
		size, size,                         // width, height
		0,                                  // border width
		DefaultDepth(display, screen),      // depth
		CopyFromParent,                     // class
		DefaultVisual(display, screen),     // visual
		CWOverrideRedirect,                 // valuemask
		&window_attr                        // attributes
	);
	XMapWindow(display, window);
	XStoreName(display, window, "find-cursor");

	XClassHint *class = XAllocClassHint();
	class->res_name = "find-cursor";
	class->res_class = "find-cursor";
	XSetClassHint(display, window, class);
	XFree(class);

	// Keep the window on top
	XEvent e;
	memset(&e, 0, sizeof(e));
	e.xclient.type = ClientMessage;
	e.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
	e.xclient.display = display;
	e.xclient.window = window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = 1;
	e.xclient.data.l[1] = XInternAtom(display, "_NET_WM_STATE_STAYS_ON_TOP", False);
	XSendEvent(display, XRootWindow(display, screen), False, SubstructureRedirectMask, &e);

	XRaiseWindow(display, window);
	XFlush(display);

	// Prepare to draw on this window
	XGCValues values = { .graphics_exposures = False };
	unsigned long valuemask = 0;
	GC gc = XCreateGC(display, window, valuemask, &values);

	Colormap colormap = DefaultColormap(display, screen);
	XColor color;
	XAllocNamedColor(display, colormap, color_name, &color, &color);
	XSetForeground(display, gc, color.pixel);
	XSetLineAttributes(display, gc, line_width, LineSolid, CapButt, JoinBevel);

	// Draw the circles
	for (int i=1; i<=size; i+=step) { 
		XDrawArc(display, window, gc,
			size/2 - i/2, size/2 - i/2,   // x, y position
			i, i,                         // Size
			0, 360 * 64);                 // Make it a full circle

		XSync(display, False);
		usleep(speed * 100);
	}
	XFreeGC(display, gc);
	XCloseDisplay(display);
}


/*
 *  The MIT License (MIT)
 * 
 *  Copyright © 2015 Martin Tournoij
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 * 
 *  The software is provided "as is", without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall the
 *  authors or copyright holders be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 */
