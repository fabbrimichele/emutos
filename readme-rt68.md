# Build
```shell
make rt68
```

# How to start the console
There are to options to to use EmuCON2

## Compile the ROM without AES and EmuCON2
In the makefile configure the following two lines:
rt68: WITH_AES = 0		# Either Graphic display, Ctrl+z to start console
rt68: WITH_CLI = 1		# or console

## Compile the ROM with AES
In the makefile configure the following two lines:
rt68: WITH_AES = 1		# Either Graphic display, Ctrl+z to start console
rt68: WITH_CLI = 0		# or console

Once the graphic UI has started, press `CTRL`+`Z`

## Mouse emulation with keyboard:
* ALT + arrow keys -> move mouse
* ALT + Insert -> left mouse button
* ALT + Home -> right mouse button

# Useful infos
* https://github.com/emutos/emutos/compare/master...aslak3:emutos:master
* https://github.com/emutos/emutos/pull/29/files
* https://sourceforge.net/p/emutos/mailman/emutos-devel/thread/5100478E.2050207%40ist-schlau.de/#msg30391800

#ROM boot address
The ROM image of EmutTOS contains in the first 4 bytes the SP and in the following 4 the PC.
That should be compatible with RT68 ROM boot mapping performed by the GLUE CPLD.


# Tested programs running under EmuCON2
* **unzip.ttp** works fine
* **stevie.tpp** works except for 
    * the arrow keys, probably because of the keyboard driver
    * you can use `h`, `l`, `j`, `k` as keys (only when not editing)
    * uses 40 columns instead of 80
* **XYZ.TTP** doesn't work, it could be because of the serial driver


# TODO
* implement serial driver
* implement mouse and keyboard drivers

## Keyboard support
* The serial interrupt vector sends received characters to `call_ikbdraw`

# Bugs

