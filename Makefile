CXX = g++
CXXFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/include
LDFLAGS = -lglib-2.0 -lnotify
TARGET = demon
SRC = demon.cpp

all: demon