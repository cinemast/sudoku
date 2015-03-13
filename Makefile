#!/usr/bin/make
#
# This makefile and all associated files has been placed into
# the public domain by Michael Kennett (July 2005), and can be
# used freely by anybody for any purpose.

DESTDIR ?= /
PREFIX ?= /usr/local

sudoku: sudoku.c
	$(CC) -o sudoku $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) sudoku.c -lcurses

clean:
	rm -f sudoku

install: sudoku sudoku.6
	install -d $(DESTDIR)$(PREFIX)/games
	install -d $(DESTDIR)$(PREFIX)/share/sudoku
	install -d $(DESTDIR)$(PREFIX)/share/man/man6
	install -m 755 sudoku $(DESTDIR)$(PREFIX)/games/sudoku
	install -m 644 template $(DESTDIR)$(PREFIX)/share/sudoku/template
	install -m 644 sudoku.6 $(DESTDIR)$(PREFIX)/share/man/man6/sudoku.6
