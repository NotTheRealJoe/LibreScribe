# LibreScribe makefile

CC = gcc
LD = g++

COMMONFLAGS =  -Winvalid-pch -include wx_pch.h
CFLAGS = -std=c++11 -Wall `wx-config --cflags` $(COMMONFLAGS) `python-config --includes` `pkg-config --cflags glib-2.0` -DWX_PRECOMP `pkg-config --cflags libxml-2.0` `pkg-config --cflags libusb-1.0` -Iinclude
LIBDIR =
LDFLAGS = `wx-config --libs` $(COMMONFLAGS) `python-config --libs` `pkg-config --libs glib-2.0` `pkg-config --libs libxml-2.0` `pkg-config --libs openobex` `pkg-config --libs libudev` `pkg-config --libs libusb-1.0`

CFLAGS_DEBUG = $(CFLAGS) -O3 -g
LIBDIR_DEBUG = $(LIBDIR)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = ./obj/Debug
DEP_DEBUG =
OUT_DEBUG = ./bin/Debug/LibreScribe

CFLAGS_RELEASE = $(CFLAGS) -O3
LIBDIR_RELEASE = $(LIBDIR)
LDFLAGS_RELEASE = -s $(LDFLAGS)
OBJDIR_RELEASE = ./obj/Release
DEP_RELEASE =
OUT_RELEASE = ./bin/Release/LibreScribe

# Targets for OBJ_
# src/AboutDialog.o
# src/DeviceInfo.o
# src/GUIFrame.o
# src/LibreScribe.o
# src/BackgroundMonitor.o
# src/Smartpen.o

OBJ_DEBUG = $(OBJDIR_DEBUG)/src/AboutDialog.o $(OBJDIR_DEBUG)/src/DeviceInfo.o $(OBJDIR_DEBUG)/src/GUIFrame.o $(OBJDIR_DEBUG)/src/LibreScribe.o $(OBJDIR_DEBUG)/src/BackgroundMonitor.o $(OBJDIR_DEBUG)/src/Smartpen.o
OBJ_RELEASE = $(OBJDIR_RELEASE)/src/AboutDialog.o $(OBJDIR_RELEASE)/src/DeviceInfo.o $(OBJDIR_RELEASE)/src/GUIFrame.o $(OBJDIR_RELEASE)/src/LibreScribe.o $(OBJDIR_RELEASE)/src/BackgroundMonitor.o $(OBJDIR_RELEASE)/src/Smartpen.o

all: before_build build_debug build_release after_build

clean: clean_debug clean_release

before_build:

after_build:
	find ./bin/ -mindepth 1 -maxdepth 1 -type d | xargs -n 1 cp -rfv -L ./res
	find ./bin/ -mindepth 1 -maxdepth 1 -type d | xargs -n 1 cp -fv -L ./stf.py ./convert_stfs.sh

before_debug:
	mkdir -p ./bin/Debug
	mkdir -p $(OBJDIR_DEBUG)/src

after_debug:

build_debug: before_debug out_debug after_debug

debug: before_build build_debug after_build

out_debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) -o $(OUT_DEBUG) $(OBJ_DEBUG) $(LDFLAGS_DEBUG) $(LIBDIR_DEBUG)

$(OBJDIR_DEBUG)/src/AboutDialog.o: src/AboutDialog.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/AboutDialog.o $(CFLAGS_DEBUG) -c src/AboutDialog.cc

$(OBJDIR_DEBUG)/src/DeviceInfo.o: src/DeviceInfo.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/DeviceInfo.o $(CFLAGS_DEBUG) -c src/DeviceInfo.cc

$(OBJDIR_DEBUG)/src/GUIFrame.o: src/GUIFrame.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/GUIFrame.o $(CFLAGS_DEBUG) -c src/GUIFrame.cc

$(OBJDIR_DEBUG)/src/LibreScribe.o: src/LibreScribe.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/LibreScribe.o $(CFLAGS_DEBUG) -c src/LibreScribe.cc

$(OBJDIR_DEBUG)/src/BackgroundMonitor.o: src/BackgroundMonitor.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/BackgroundMonitor.o $(CFLAGS_DEBUG) -c src/BackgroundMonitor.cc

$(OBJDIR_DEBUG)/src/Smartpen.o: src/Smartpen.cc
	$(CC) -o $(OBJDIR_DEBUG)/src/Smartpen.o $(CFLAGS_DEBUG) -c src/Smartpen.cc

clean_debug:
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -fr ./bin/Debug
	rm -fr $(OBJDIR_DEBUG)/src

before_release:
	mkdir -p ./bin/Release
	mkdir -p $(OBJDIR_RELEASE)/src

after_release:

build_release: before_release out_release after_release

release: before_build build_release after_build

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) -o $(OUT_RELEASE) $(OBJ_RELEASE) $(LDFLAGS_RELEASE) $(LIBDIR_RELEASE)

$(OBJDIR_RELEASE)/src/AboutDialog.o: src/AboutDialog.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/AboutDialog.o $(CFLAGS_RELEASE) -c src/AboutDialog.cc

$(OBJDIR_RELEASE)/src/DeviceInfo.o: src/DeviceInfo.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/DeviceInfo.o $(CFLAGS_RELEASE) -c src/DeviceInfo.cc

$(OBJDIR_RELEASE)/src/GUIFrame.o: src/GUIFrame.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/GUIFrame.o $(CFLAGS_RELEASE) -c src/GUIFrame.cc

$(OBJDIR_RELEASE)/src/LibreScribe.o: src/LibreScribe.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/LibreScribe.o $(CFLAGS_RELEASE) -c src/LibreScribe.cc

$(OBJDIR_RELEASE)/src/BackgroundMonitor.o: src/BackgroundMonitor.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/BackgroundMonitor.o $(CFLAGS_RELEASE) -c src/BackgroundMonitor.cc

$(OBJDIR_RELEASE)/src/Smartpen.o: src/Smartpen.cc
	$(CC) -o $(OBJDIR_RELEASE)/src/Smartpen.o $(CFLAGS_RELEASE) -c src/Smartpen.cc

clean_release:
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -fr ./bin/Release
	rm -fr $(OBJDIR_RELEASE)/src

.PHONY: before_build after_build before_debug after_debug clean_debug before_release after_release clean_release
