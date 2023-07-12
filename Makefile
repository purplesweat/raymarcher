CC=cc
CFLAGS=$$(pkg-config --cflags gtk4 gdk-pixbuf-2.0) -O2
LDFLAGS=$$(pkg-config --libs gtk4 gdk-pixbuf-2.0) -lm

SOURCE=src
PROGNAME=raymarcher
PREFIX=/usr/local

.PHONY: install

$(PROGNAME): $(SOURCE)/main.c
	$(CC) $(CFLAGS) -o $(PROGNAME) $(SOURCE)/main.c $(LDFLAGS)
	strip $(PROGNAME)

install: $(PROGNAME)
	cp $(PROGNAME) $(PREFIX)/bin
