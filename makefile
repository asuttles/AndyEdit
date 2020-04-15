vpath %.c src
vpath %.h src

# Source Code
SRC=ae.c keyPress.c minibuffer.c statusBar.c
CFLAGS=-Wall -Wextra -pedantic -std=c99

# Object Files
OBJS=$(SRC:.c=.o)

# Libraries
LIBS=-lcurses -lreadline

# Build Files
ae: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)
	mv *.o obj

%.o: %.c %.h ae.h
	$(CC) -o $@ -c $(CFLAGS) $<

# Header Dependencies
ae.o : keyPress.h minibuffer.h statusBar.h
keyPress.o : ae.h
minibuffer.o : ae.h keyPress.h
statusBar.o : ae.h

# Targets
.phony: install

install: ae 
	mv ae ~/bin
