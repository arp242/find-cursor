/*
 * http://code.arp242.net/find-cursor
 * Copyright © Martin Tournoij <martin@arp242.net>
 * See below for full copyright
 */

#define _DEFAULT_SOURCE

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
int pointer_screen(char *name, Display *display);
void draw(char *name, Display *display, int screen,
	int size, int distance, int wait, int line_width, char *color_name,
	int follow, int transparent, int grow, int outline, char *ocolor_name);

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
	{"outline",       optional_argument, NULL, 'o'}, // Optional for compat, as previously it was hard-coded to 2px.
	{"repeat",        required_argument, NULL, 'r'},
	{"outline-color", required_argument, NULL, 'O'},
	{NULL, 0, NULL, 0}
};

void usage(char *name) {
	printf(""
	"find-cursor highlights the cursor position by drawing circles around it.\n"
	"https://github.com/arp242/find-cursor\n"
	"\n"
	"Flags:\n"
	"  -h, --help          Show this help.\n"
	"\n"
	"Shape options:\n"
	"  -s, --size          Maximum size the circle will grow to in pixels.\n"
	"  -d, --distance      Distance between the circles in pixels.\n"
	"  -l, --line-width    Width of the lines in pixels.\n"
	"  -w, --wait          Time to wait before drawing the next circle in\n"
	"                      tenths of milliseconds.\n"
	"  -c, --color         Color as X11 color name or RGB (e.g. #ff0000)\n"
	"  -g, --grow          Grow the animation in size, rather than shrinking it.\n"
	"\n"
	"Extra options:\n"
	"  -f, --follow        Follow the cursor position as the cursor is moving.\n"
	"  -t, --transparent   Make the window truly 'transparent'. This helps with\n"
	"                      some display issues when following the cursor position,\n"
	"                      but it doesn't work well with all WMs, which is why\n"
	"                      it's disabled by default.\n"
	"  -o, --outline       Width in pixels of outline; uses 2px if no value is given.\n"
	"                      Helps visibility on all backgrounds.\n"
	"  -O, --outline-color Color of outline; if omitted it will automatically use\n"
	"                      the opposite color. No effect if -o isn't set.\n"
	"  -r, --repeat        Number of times to repeat the animation; use 0 to repeat\n"
	"                      indefinitely.\n"
	"\n"
	"Examples:\n"
	"  The defaults:\n"
	"    %s --size 320 --distance 40 --wait 400 --line-width 4 --color black\n\n"
	"  Draw a solid circle:\n"
	"    %s --size 100 --distance 1 --wait 20 --line-width 1\n\n"
	"  Always draw a full circle on top of the cursor:\n"
	"    %s --repeat 0 --follow --distance 1 --wait 1 --line-width 16 --size 16\n"
	"\n"
	"Launching:\n"
	"  You will want to map a key in your window manager to run find-cursor.\n"
	"  You can also use xbindkeys, which should work with any window manager.\n"
	"\n"
	"  I run it with xcape:\n"
	"       xcape -e 'Control_L=Escape;Shift_L=KP_Add'\n"
	"\n"
	"  When Left Shift is tapped a Keypad Add is sent; I configured my window\n"
	"  manager to launch find-cursor with that.\n"
	"\n"
	"  I don't have a numpad on my keyboard; you can also use F13 or some\n"
	"  other unused key.\n",
	name, name, name);
}

// Parse number from commandline, or show error and exit if it's not a number.
int parse_num(int ch, char *opt, char *name) {
	char *end;
	long result = strtol(opt, &end, 10);
	if (*end) {
		fprintf(stderr, "%s: value for -%c must be a number\n\n", name, ch);
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
	char ocolor_name[64];
	int follow = 0;
	int transparent = 0;
	int grow = 0;
	int outline = 0;
	int repeat = 0;

	extern int optopt;
	int ch;
	while ((ch = getopt_long(argc, argv, ":hs:d:w:l:c:r:ftgo:O:", longopts, NULL)) != -1)
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
			outline = parse_num(ch, optarg, argv[0]);
			break;
		case 'O':
			strncpy(ocolor_name, optarg, sizeof(ocolor_name));
			break;
		case 'r':
			repeat = parse_num(ch, optarg, argv[0]);
			if (repeat == 0)
				repeat = -1;
			break;
		case ':':
			switch (optopt) {
			case 'o':
				outline = 2;
				break;
			default:
			    fprintf(stderr, "%s: missing required argument for -%c\n\n", argv[0], optopt);
			    usage(argv[0]);
			    exit(1);
			}
			break;
		case '?':
			fprintf(stderr, "%s: invalid option: -%c\n\n", argv[0], ch);
			// fallthrough
		default:
			usage(argv[0]);
			exit(1);
		}

	// Setup display and such
	char *display_name = getenv("DISPLAY");
	if (!display_name) {
		fprintf(stderr, "%s: DISPLAY not set\n\n", argv[0]);
		exit(1);
	}

	Display *display = XOpenDisplay(display_name);
	if (!display) {
		fprintf(stderr, "%s: cannot open display '%s'\n\n", argv[0], display_name);
		exit(1);
	}

	int shape_event_base, shape_error_base;
	if (!XShapeQueryExtension(display, &shape_event_base, &shape_error_base)) {
		fprintf(stderr, "%s: no XShape extension for display '%s'\n\n", argv[0], display_name);
		exit(1);
	}

	// Actually draw.
	do
		draw(argv[0], display, pointer_screen(argv[0], display),
			size, distance, wait, line_width, color_name,
			follow, transparent, grow, outline, ocolor_name);
	while (repeat == -1 || repeat--);

	XCloseDisplay(display);
}

// Identify which screen the cursor is on.
int pointer_screen(char *name, Display *display) {
	int screencount = ScreenCount(display);

	// The traditional case
	if (screencount == 1)
		return DefaultScreen(display);

	// Multihead
	for (int s=0; s < screencount; s++) {
		Window root, win;
		int unused           = 0;
		unsigned int unusedu = 0;
		Screen *screen       = ScreenOfDisplay(display, s);

		int found = XQueryPointer(display, RootWindowOfScreen(screen),
			&root, &win, &unused, &unused, &unused, &unused, &unusedu);
		if (found)
			return s;
	}

	// Fall through (should never happen)
	fprintf(stderr, "%s: Unable to identify pointer screen, using Default\n", name);
	return DefaultScreen(display);
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
	int follow, int transparent, int grow, int outline, char *ocolor_name
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

	// Get colours.
	Colormap colormap = DefaultColormap(display, screen);
	XColor color;
	int err = XAllocNamedColor(display, colormap, color_name, &color, &color);
	if (err == 0) {
		fprintf(stderr, "%s: invalid color value for -c/--color: '%s'\n\n", name, color_name);
		usage(name);
		exit(1);
	}

	XColor color2;
	if (outline > 0) {
		if (ocolor_name[0] == 0) { // Use opposite colour.
			color2.red   = 65535 - color.red;
			color2.green = 65535 - color.green;
			color2.blue  = 65535 - color.blue;
			sprintf(ocolor_name, "#%04X%04X%04X", color2.red, color2.green, color2.blue);
		}
		int err = XAllocNamedColor(display, colormap, ocolor_name, &color2, &color2);
		if (err == 0) {
			fprintf(stderr, "%s: invalid color value for -O/--outline-color: '%s'\n\n", name, ocolor_name);
			usage(name);
			exit(1);
		}
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

		if (outline > 0) {
			XSetLineAttributes(display, gc, line_width+outline, LineSolid, CapButt, JoinBevel);
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


/* The MIT License (MIT)
 *
 * Copyright © Martin Tournoij
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The software is provided "as is", without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings
 * in the software. */
