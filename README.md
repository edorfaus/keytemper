KeyTemper
=========

This is a program for reading the sensor values from the HID variants of the
TEMPer series of USB temperature and humidity measurement devices.

To be specific, it supports those devices that include the direct-to-Excel
feature, where you press and hold Num Lock for a few seconds, and the device
starts sending keypresses that fill out an Excel sheet with the measurements.

AFAIK, this includes most (or maybe all) of the newer variants, but does *not*
include any of the (mostly older) devices that emulate a serial port.

Of note is that the above-mentioned feature generally does not work on Linux,
despite how it is advertised, because the devices assume that a particular
aspect of the system's keyboard handling behaviour is the same as how Windows
does it, while Linux handles this particular aspect differently.

Hence, this program emulates that aspect of how Windows does it, but also grabs
the device so that it can read the keypresses, and preferably does so in such
a way that the system does not see those keypresses (as that would make it
difficult to use the system for anything else while this was going on).


Notes about the current code
----------------------------

At the moment, the code is quite simple, and doesn't do very much. It's
basically a proof of concept that I quickly wrote and then cleaned up a bit.

Currently, it starts the device, reads a specified number of lines from it, and
then stops the device again. The keypresses it reads in between are mapped into
characters which are then printed on the standard output of the program.

This means that you can, in fact, get the measurements using this program in
its current state, simple though it may be, but only on the format used by the
device - and you must know the device path for your device.

### Finding the device path

While I intend to add it eventually, there is currently no enumeration of the
available devices being done by this program, so you must find the device path
to your device on your own.

One way of doing this would be to use the hid-query program from [tempered][1],
_when that is compiled with libusb instead of hidraw_ (as their paths differ).

Or, you could put it together manually - while this depends on the internals of
HIDAPI, mine looks like `0003:001c:00`, which appears to be the USB bus, the
USB device number on that bus, and the number of the interface on that device,
separated by colons, all in hexadecimal. The first two can be found with
`lsusb` (except in decimal) while the last is always 0 for the devices I have.

Worth noting is that this is the _other_ interface than what tempered uses.

[1]: https://github.com/edorfaus/TEMPered


Compiling
---------

This project requires a recent version of [HIDAPI][2]. You'll probably need to
tell the build system where to find it, either by editing the Makefile or by
providing the path on the commandline, like this: `make HIDAPI_DIR=../hidapi/`

(Yes, I should change this to at least look in standard system locations.)

By default, it will use the libusb variant of HIDAPI, which is the recommended
one, and will link against the shared library.

While the hidraw variant also works, it will not detach the device from the
system's input subsystem, which means that the keypresses will also show up on
your system as a whole, which is probably not what you want.

[2]: http://www.signal11.us/oss/hidapi/
