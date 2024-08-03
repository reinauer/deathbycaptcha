Death By Captcha API binary client
==================================


To build the Death By Captcha API client, whether shared library or console
util, you'll need a decent GNU toolchain, at least make and GCC or clang.
For Windows users we recommend mingw32/MSYS package -- we build our own
binaries with it.


How to build
------------

Notes:

1. When building for Windows, define WIN32 build flag and set it to 1 (just
add WIN32=1 right after `make`).

2. Make sure the sources are clean by running `make clean` before building
targets.


To build the stand-alone console client, run:

    make client


You'll find the compiled binary in the 'bin' directory, named 'deathbycaptcha'
under Linux, or 'deathbycaptcha.exe' under Windows.


To build the dynamic library, run:

    make lib

You'll find the compiled library in the 'lib' directory, named
'libdeathbycaptcha.so' under Linux, or 'deathbycaptcha.dll' under Windows
(accompanied with 'deathbycaptcha.lib' import library).


Windows users have an option to build a plugin for MyAdTools bots.
To do so, run:

    make myadtools_lib

You'll find the compiled plugin in the 'lib/MyAdTools' directory, named
'DeathByCaptcha.dll'.  It is supposed to work as a drop-in replacement
for the plugin distributed by MyAdTools with the only caveat: you'll have
to rename the DBCSetings.txt file to the properly spelled DBCSettings.txt.
