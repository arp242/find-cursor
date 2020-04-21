/*
 * http://code.arp242.net/find-cursor
 * Copyright © 2015-2020 Martin Tournoij <martin@arp242.net>
 * See below for full copyright
 */

#define _XOPEN_SOURCE 500

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>

void usage(char *name);
int parse_num(int ch, char *opt, char *name);
void draw(char *name, Display *display, int screen,
	int size, int distance, int wait, int line_width, char *color_name,
	int follow, int transparent, int grow, int outline, int repeat);

static struct option longopts[] = {
	{"help",          no_argument,       NULL, 'h'},
	{"size",          required_argument, NULL, 's'},
	{"distance",      required_argument, NULL, 'd'},
	{"wait",          required_argument, NULL, 'w'},
	{"line-width",    required_argument, NULL, 'l'},
	{"color",         required_argument, NULL, 'c'},
	{"follow",        no_argument,       NULL, 'f'},
	{"transparent",   no_argument,       NULL, 't'},
	{"grow",          no_argument,       NULL, 'g'},
	{"outline",       no_argument,       NULL, 'o'},
	{"repeat",        required_argument, NULL, 'r'},
	{NULL, 0, NULL, 0}
};

void usage(char *name) {
	printf("Usage: %s [-stplcftgr]\n", name);
	printf("\n");
	printf("Flags:\n");
	printf("  -h, --help          Show this help.\n");
	printf("\n");
	printf("Shape options:\n");
	printf("  -s, --size          Maximum size the circle will grow to in pixels.\n");
	printf("  -d, --distance      Distance between the circles in pixels.\n");
	printf("  -l, --line-width    Width of the lines in pixels.\n");
	printf("  -w, --wait          Time to wait before drawing the next circle in\n");
	printf("                      tenths of milliseconds.\n");
	printf("  -c, --color         Color as X11 color name or RGB (e.g. #ff0000)\n");
	printf("  -g, --grow          Grow the animation in size, rather than shrinking it.\n");
	printf("\n");
	printf("Extra options:\n");
	printf("  -f, --follow        Follow the cursor position as the cursor is moving.\n");
	printf("  -t, --transparent   Make the window truly 'transparent'. This helps with\n");
	printf("                      some display issues when following the cursor position,\n");
	printf("                      but it doesn't work well with all WMs, which is why\n");
	printf("                      it's disabled by default.\n");
	printf("  -o, --outline       Draw an outline in the opposite color as well. Helps\n");
	printf("                      visibility on all backgrounds.\n");
	printf("  -r, --repeat        Number of times to repeat the animation; use 0 to repeat\n");
	printf("                      indefinitely.\n");
	printf("\n");
	printf("Examples:\n");
	printf("  The defaults:\n");
	printf("    %s --size 320 --distance 40 --wait 400 --line-width 4 --color black\n\n", name);
	printf("  Draw a solid circle:\n");
	printf("    %s --size 100 --distance 1 --wait 20 --line-width 1\n\n", name);
	printf("  Always draw a full circle on top of the cursor:\n");
	printf("    %s --repeat 0 --follow --distance 1 --wait 1 --line-width 16 --size 16\n", name);
}

// Parse number from commandline, or show error and exit if it's not a number.
int parse_num(int ch, char *opt, char *name) {
	char *end;
	long result = strtol(optarg, &end, 10);
	if (*end) {
		fprintf(stderr, "%s: %d must be a number\n", name, ch);
		usage(name);
		exit(1);
	}
	return result;
}

int main(int argc, char* argv[]) {
	// Parse options
	int size = 320;
	int distance = 40;
	int wait = 400;
	int line_width = 4;
	char color_name[64] = "black";
	int follow = 0;
	int transparent = 0;
	int grow = 0;
	int outline = 0;
	int repeat = 0;

	int ch;
	while ((ch = getopt_long(argc, argv, "hs:d:w:l:c:r:ftgo", longopts, NULL)) != -1)
		switch (ch) {
		case 's':
			size = parse_num(ch, optarg, argv[0]);
			break;
		case 'd':
			distance = parse_num(ch, optarg, argv[0]);
			break;
		case 'w':
			wait = parse_num(ch, optarg, argv[0]);
			break;
		case 'l':
			line_width = parse_num(ch, optarg, argv[0]);
			break;
		case 'c':
			strncpy(color_name, optarg, sizeof(color_name));
			break;
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'f':
			follow = 1;
			break;
		case 't':
			transparent = 1;
			break;
		case 'g':
			grow = 1;
			break;
		case 'o':
			outline = 1;
			break;
		case 'r':
			repeat = parse_num(ch, optarg, argv[0]);
			if (repeat == 0)
				repeat = -1;
			break;
		default:
			usage(argv[0]);
			exit(1);
		}

	// Setup display and such
	char *display_name = getenv("DISPLAY");
	if (!display_name) {
		fprintf(stderr, "%s: DISPLAY not set\n", argv[0]);
		exit(1);
	}

	Display *display = XOpenDisplay(display_name);
	if (!display) {
		fprintf(stderr, "%s: cannot open display '%s'\n", argv[0], display_name);
		exit(1);
	}
	int screen = DefaultScreen(display);

	int shape_event_base, shape_error_base;
	if (!XShapeQueryExtension(display, &shape_event_base, &shape_error_base)) {
		fprintf(stderr, "%s: no XShape extension for display '%s'\n", argv[0], display_name);
		exit(1);
	}

	// Actually draw.
	do
		draw(argv[0], display, screen,
			size, distance, wait, line_width, color_name,
			follow, transparent, grow, outline, repeat);
	while (repeat == -1 || repeat--);

	XCloseDisplay(display);
}

// Try to get the centre of the cursor.
void cursor_center(Display *display, int size, int *x, int *y) {
	XFixesCursorImage *c = XFixesGetCursorImage(display);
	*x = c->x - size/2 - c->width/2  + c->xhot;
	*y = c->y - size/2 - c->height/2 + c->yhot;
	XFree(c);
}

void draw(char *name, Display *display, int screen,
	int size, int distance, int wait, int line_width, char *color_name,
	int follow, int transparent, int grow, int outline, int repeat
) {
	// Get the mouse cursor position and size.
	int center_x, center_y;
	cursor_center(display, size, &center_x, &center_y);

	// Create a window at the mouse position
	Window window;
	XSetWindowAttributes window_attr;
	window_attr.override_redirect = 1;

	if (transparent) {
		XVisualInfo vinfo;
		XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo);
		window_attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
		window_attr.background_pixel = 0;
		window = XCreateWindow(display, XRootWindow(display, screen),
			center_x,                               // x position
			center_y,                               // y position
			size, size,                             // width, height
			4,                                      // border width
			vinfo.depth,                            // depth
			CopyFromParent,                         // class
			vinfo.visual,                           // visual
			CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, // valuemask
			&window_attr);                          // attributes
	}
	else
		window = XCreateWindow(display, XRootWindow(display, screen),
			center_x,                              // x position
			center_y,                              // y position
			size, size,                            // width, height
			4,                                     // border width
			DefaultDepth(display, screen),         // depth
			CopyFromParent,                        // class
			DefaultVisual(display, screen),        // visual
			CWOverrideRedirect,                    // valuemask
			&window_attr);                         // attributes

	// Make round shaped window.
	XGCValues xgcv;
	Pixmap shape_mask = XCreatePixmap(display, window, size, size, 1);
	GC part_shape = XCreateGC(display, shape_mask, 0, &xgcv);
	XSetForeground(display, part_shape, 0);
	XFillRectangle(display, shape_mask, part_shape, 0, 0, size, size);
	XSetForeground(display, part_shape, 1);
	XFillArc(display, shape_mask, part_shape,
		0, 0,         // x, y position
		size, size,   // Size
		0, 360 * 64); // Make it a full circle
	XFreeGC(display, part_shape);

	XShapeCombineMask(display, window, ShapeBounding, 0, 0, shape_mask, ShapeSet);
	XShapeCombineMask(display, window, ShapeClip,     0, 0, shape_mask, ShapeSet);
	XFreePixmap(display, shape_mask);

	// Input region is small so we can pass input events.
	XRectangle rect;
	XserverRegion region = XFixesCreateRegion(display, &rect, 1);
	XFixesSetWindowShapeRegion(display, window, ShapeInput, 0, 0, region);
	XFixesDestroyRegion(display, region);

	// Set attrs
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

	// Make sure compositing window managers don't apply shadows and such.
	// https://specifications.freedesktop.org/wm-spec/1.4/ar01s05.html
	// https://wiki.archlinux.org/index.php/Compton
	Atom type_util = XInternAtom(display, "_NET_WM_WINDOW_TYPE", 0);
	Atom type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_MENU", 0);
	XChangeProperty(display, window, type, XA_ATOM, 32, PropModeReplace,
		(unsigned char *)&type_util, 1);

	XRaiseWindow(display, window);
	XFlush(display);

	// Prepare to draw on this window
	XGCValues values = { .graphics_exposures = False };
	unsigned long valuemask = 0;
	GC gc = XCreateGC(display, window, valuemask, &values);

	Colormap colormap = DefaultColormap(display, screen);
	XColor color;
	XAllocNamedColor(display, colormap, color_name, &color, &color);

	XColor color2;
	char color2_name[14]; // hash + 3x4-digit hex
	if (outline) {
		// Insert and convert to XColor.
		color2.red   = 65535 - color.red;
		color2.green = 65535 - color.green;
		color2.blue  = 65535 - color.blue;
		sprintf(color2_name, "#%04X%04X%04X", color2.red, color2.green, color2.blue);
		XAllocNamedColor(display, colormap, color2_name, &color2, &color2);
	}
	else {
		// Set colour only once if not outline.
		XSetLineAttributes(display, gc, line_width, LineSolid, CapButt, JoinBevel);
		XSetForeground(display, gc, color.pixel);
	}

	// Draw the circles
	int i = 1;
	for (i=1; i<=size; i+=distance) {
		if (follow)
			XClearWindow(display, window);

		int cs = grow ? i : size - i;

		if (outline) {
			XSetLineAttributes(display, gc, line_width+2, LineSolid, CapButt, JoinBevel);
			XSetForeground(display, gc, color2.pixel);
			XDrawArc(display, window, gc,
				size/2 - cs/2, size/2 - cs/2, // x, y position
				cs, cs,                       // Size
				0, 360 * 64);                 // Make it a full circle

			// Set color back for the normal circle.
			XSetLineAttributes(display, gc, line_width, LineSolid, CapButt, JoinBevel);
			XSetForeground(display, gc, color.pixel);
		}

		XDrawArc(display, window, gc,
			size/2 - cs/2, size/2 - cs/2, // x, y position
			cs, cs,                       // Size
			0, 360 * 64);                 // Make it a full circle

		if (follow) {
			cursor_center(display, size, &center_x, &center_y);
			XMoveWindow(display, window, center_x, center_y);
		}

		XSync(display, False);
		usleep(wait * 100);
	}

	XFreeGC(display, gc);
	XDestroyWindow(display, window);
}


/*
 *  The MIT License (MIT)
 *
 *  Copyright © 2015-2020 Martin Tournoij
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
