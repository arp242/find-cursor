Project status: stable

-----------------------------------------

Simple XLib program to highlight the cursor position. This is similar to the
feature found in Windows XP (and possibly later?)

![screenshot.gif](https://bytebucket.org/Carpetsmoker/find-cursor/raw/tip/screenshot.gif)

Using it
========
Compile it by typing `make`. Run it with `-h` for some options controlling the
appearance.

- The author runs it with [`xcape`][xcape]:

		xcape -e 'Control_L=Escape;Shift_L=KP_Add'  

	When Left shift is tapped, a Keypad Add is sent – I don't have a numpad on my
	keyboard – which we can then use to launch the program.

- I configured my window manager (PekWM) to pick up Numpad Add and launch this:

		KeyPress = "KP_Add" { Actions = "Exec find-cursor" }

	I'm not going to include instructions for every window manager out there.
	I'm sure you can figure out how to use it with your WM ;-) You can also use
	[`xbindkeys`](xbindkeys), which should work with `$any` window manager.

[xcape]: https://github.com/alols/xcape
[xbindkeys]: http://www.nongnu.org/xbindkeys/xbindkeys.html
