#!/usr/bin/make
#
# This makefile and all associated files has been placed into
# the public domain and can be used freely by anybody for any purpose.

DEFINES=-DTEMPLATE=\"template\" -DPRECANNED=\"precanned\"
CFLAGS+=$(DEFINES)
ROOT_DIR=..
CURR_DIR=.
WIN32_DIR=$(ROOT_DIR)/win32

CURSES_ON_WIN32=$(WIN32_DIR)/curses.c  \
                $(WIN32_DIR)/termios.c \
                $(WIN32_DIR)/unistd.c

CURSES_ON_WIN32_INCLUDE=$(WIN32_DIR)/curses.h  \
                        $(WIN32_DIR)/termios.h \
                        $(WIN32_DIR)/unistd.h

SRC_FILES= $(ROOT_DIR)/sudoku.c \
           $(CURSES_ON_WIN32)

CC=gcc
OPTFLAGS=-O2

COMP_FLAGS= $(LDFLAGS)  \
            $(CPPFLAGS) \
            $(CFLAGS)   \
            $(OPTFLAGS)

INCLUDE_DIRS= -I $(WIN32_DIR)
OUT_DIR=$(CURR_DIR)/bin
OUT_FILE=$(OUT_DIR)/sudoku.exe


sudoku: $(ROOT_DIR)/sudoku.c    \
        $(CURSES_ON_WIN32)      \
        $(CURSES_ON_WIN32_INCLUDE)
	$(CC) -o $(OUT_FILE) $(INCLUDE_DIRS) $(COMP_FLAGS) $(SRC_FILES)

clean:
	del $(OUT_FILE)
