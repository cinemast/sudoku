#!/usr/bin/make
#
# This makefile and all associated files has been placed into
# the public domain by Michael Kennett (July 2005), and can be
# used freely by anybody for any purpose.

OWN=-o root
GRP=-g root
DESTDIR=/

sudoku: sudoku.c
	$(CC) -o sudoku $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) sudoku.c -lcurses

clean:
	rm -f sudoku

install: sudoku sudoku.6
	install $(OWN) $(GRP) -m 755 sudoku $(DESTDIR)/usr/games/sudoku
	install -d $(OWN) $(GRP) -m 755 $(DESTDIR)/usr/share/sudoku
	install $(OWN) $(GRP) -m 644 template $(DESTDIR)/usr/share/sudoku/template
	install $(OWN) $(GRP) -m 644 sudoku.6 $(DESTDIR)/usr/share/man/man6/sudoku.6

