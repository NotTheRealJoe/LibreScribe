# Build Instructions
## Tools
You need GNU Make, GCC, G++, pkg-config, and Python 2.7 (with python-config) to build LibreScribe.

On Debian or Ubuntu:
```
apt install make gcc g++ pkg-config python-dev
```

Python is used for the `stf.py` script. Not sure if this is actually needed for the main program yet.

## Shared Libraries
The following shared libraries are needed to build LibreScribe
wx, libxml-2.0, libusb-1.0, libudev, glib, OpenOBEX, and libusb

On Debian or Ubuntu they can be installed with:
```
apt install libwxgtk3.0-gtk3-dev libxml2-dev libusb-1.0-0-dev libudev-dev libopenobex2-dev libglib2.0-dev libusb-dev
```
On some versions `libwxgtk3.0-gtk3-dev` is instead called `libwxgtk3.0-dev`

*`libwxgtk3.0-gtk3-dev` is also used to provide the `wx-config` command when building*

## Building
Once the dependicies are installed, build with `make release` for a release build, or just `make` for a development
build.

Release builds are output to the `bin/Release` directory, and development builds are output to the `bin/Debug` directory