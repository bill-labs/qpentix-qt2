# Pentix for Qt2 - Makefile
# Simple Qt2 application

# Installation prefix
PREFIX ?= /opt/gnux
FONTPATH ?= $(PREFIX)/share/fonts
QTDIR ?= $(PREFIX)/qt2
MOC = $(QTDIR)/bin/moc

# FreeType configuration
FT_CFLAGS = -I$(PREFIX)/include/freetype2 -I$(PREFIX)/include
FT_LDFLAGS = -L$(PREFIX)/lib -lfreetype -lz

CXX = g++
CXXFLAGS = -O3 -fomit-frame-pointer -Wall -D_UNIXWARE -DPREFIX=\"$(PREFIX)\" -DFONTPATH_DEFAULT=\"$(FONTPATH)\" -I$(QTDIR)/include -I. $(FT_CFLAGS)
LDFLAGS = -L$(QTDIR)/lib -L$(PREFIX)/lib -lqt $(FT_LDFLAGS)

TARGET = qpentix

# Source files
SOURCES = main.cpp \
          piece.cpp \
          playfield.cpp \
          ttftext.cpp \
          HighScoreManager.cpp \
          HighScoreDialog.cpp

# Moc files
MOC_SOURCES = main.moc \
              HighScoreDialog.moc

# Objects
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.moc: main.h
	$(MOC) main.h -o main.moc

HighScoreDialog.moc: HighScoreDialog.h
	$(MOC) HighScoreDialog.h -o HighScoreDialog.moc

# Moc dependencies
main.o: main.moc
HighScoreDialog.o: HighScoreDialog.moc

clean:
	rm -f $(OBJECTS) $(MOC_SOURCES) $(TARGET) core

install:
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/

.PHONY: all clean install
