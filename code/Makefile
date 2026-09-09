# Makefile for single-file C test programs in this directory.
#
# Usage:
#   make foo           build foo.c -> foo.exe
#   make foo.exe       same as above
#   make all           build every .c found in this directory
#   make run T=foo     build (if needed) and run foo.exe
#   make clean         remove all .exe that have a matching .c
#
# Notes:
#   - Windows / MinGW: executables get the .exe suffix (EXE variable).
#   - If a test needs libm:  make foo LDLIBS=-lm

CC     := gcc
CFLAGS := -std=c11 -Wall -Wextra -O0 -g
LDLIBS :=
EXE    := .exe

SRCS    := $(wildcard *.c)
TARGETS := $(SRCS:.c=$(EXE))

.PHONY: all run clean help

all: $(TARGETS)

# pattern rule: any foo.c -> foo.exe
%$(EXE): %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

# convenience: allow "make foo" instead of "make foo.exe"
%: %$(EXE)
	@echo "built: $<"

run:
ifdef T
	./$(T)$(EXE)
else
	@echo "usage: make run T=<name>    (name without .c / .exe)"
endif

clean:
	rm -f $(TARGETS)

help:
	@echo "make <name>       build <name>.c -> <name>.exe"
	@echo "make all          build every .c here"
	@echo "make run T=<name> build and run"
	@echo "make clean        remove generated .exe"
