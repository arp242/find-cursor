[![This project is considered stable](https://img.shields.io/badge/Status-stable-green.svg)](https://arp242.net/status/stable)
[![Build Status](https://travis-ci.org/Carpetsmoker/find-cursor.svg?branch=master)](https://travis-ci.org/Carpetsmoker/find-cursor)

Simple XLib program to highlight the cursor position. This is similar to the
feature found in Windows XP (and possibly later?)

![screenshot.gif](https://raw.githubusercontent.com/Carpetsmoker/find-cursor/master/screenshot.gif)

Using it
========
Compile it by typing `make`. Run it with `-h` to see some options for
controlling the appearance.

- The author runs it with [`xcape`][xcape]:

		xcape -e 'Control_L=Escape;Shift_L=KP_Add'  

	When Left shift is tapped, a Keypad Add is sent – I don't have a numpad on my
	keyboard – which we can then use to launch the program.

- I configured my window manager (PekWM) to pick up Numpad Add and launch this:

		KeyPress = "KP_Add" { Actions = "Exec find-cursor" }

	I'm not going to include instructions for every window manager out there.
	I'm sure you can figure out how to use it with your WM ;-) You can also use
	[`xbindkeys`](xbindkeys), which should work with `$any` window manager.

- You may want to disable shadows if you use compton or some other composite
  manager; for example for compton start it with:

		compton --shadow-exclude "class_g = 'find-cursor'"

	Or, perhaps even better, disable it for all shaped windows:

		compton --shadow-exclude 'bounding_shaped'

	You can also put that in the compton config file. Other managers might have
	different options/flags.

[xcape]: https://github.com/alols/xcape
[xbindkeys]: http://www.nongnu.org/xbindkeys/xbindkeys.html
