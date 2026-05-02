CC=gcc

PREFIX=/usr/local

BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/man1

CFLAGS=\
-I/usr/include \
-O2

LDFLAGS=\
-L/usr/lib/tobii \
-Wl,-rpath,/usr/lib/tobii

LIBS=\
-ltobii_stream_engine \
-lX11 \
-lm

TARGETS=\
calibration_x11 \
eye_mouse_x11

all: $(TARGETS)

calibration_x11: calibration_x11.c
	$(CC) calibration_x11.c \
	-o calibration_x11 \
	$(CFLAGS) \
	$(LDFLAGS) \
	$(LIBS)

eye_mouse_x11: eye_mouse_x11.c
	$(CC) eye_mouse_x11.c \
	-o eye_mouse_x11 \
	$(CFLAGS) \
	$(LDFLAGS) \
	$(LIBS)

install: all
	install -d $(BINDIR)
	install -d $(MANDIR)

	install -m 755 calibration_x11 \
	$(BINDIR)/calibration_x11

	install -m 755 eye_mouse_x11 \
	$(BINDIR)/eye_mouse_x11

	install -m 644 calibration_x11.1 \
	$(MANDIR)/calibration_x11.1

	install -m 644 eye_mouse_x11.1 \
	$(MANDIR)/eye_mouse_x11.1

uninstall:
	rm -f $(BINDIR)/calibration_x11
	rm -f $(BINDIR)/eye_mouse_x11

	rm -f $(MANDIR)/calibration_x11.1
	rm -f $(MANDIR)/eye_mouse_x11.1

clean:
	rm -f $(TARGETS)
