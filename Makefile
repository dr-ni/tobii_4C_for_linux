CC      := gcc

PREFIX  ?= /usr/local
BINDIR  := $(PREFIX)/bin
MANDIR  := $(PREFIX)/share/man/man1

CFLAGS  := -O2 -Wall -Wextra -I/usr/include
LDFLAGS := -L/usr/lib/tobii -Wl,-rpath,/usr/lib/tobii

LIBS_COMMON := -ltobii_stream_engine -lX11 -lm
LIBS_EYECALIB := $(LIBS_COMMON)
LIBS_EYEMOUSE := $(LIBS_COMMON) -lXtst -lpthread

TARGETS := eyecalib eyemouse

.PHONY: all clean install uninstall

all: $(TARGETS)

eyecalib: eyecalib.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS_EYECALIB)

eyemouse: eyemouse.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS_EYEMOUSE)

clean:
	rm -f $(TARGETS)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)

	install -m 755 eyecalib \
		$(DESTDIR)$(BINDIR)/eyecalib

	install -m 755 eyemouse \
		$(DESTDIR)$(BINDIR)/eyemouse

	install -m 644 eyecalib.1 \
		$(DESTDIR)$(MANDIR)/eyecalib.1

	install -m 644 eyemouse.1 \
		$(DESTDIR)$(MANDIR)/eyemouse.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/eyecalib
	rm -f $(DESTDIR)$(BINDIR)/eyemouse

	rm -f $(DESTDIR)$(MANDIR)/eyecalib.1
	rm -f $(DESTDIR)$(MANDIR)/eyemouse.1
