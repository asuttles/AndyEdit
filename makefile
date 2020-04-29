vpath %.c src
vpath %.h src

# Source Code
SRC=ae.c keyPress.c minibuffer.c statusBar.c \
    pointMarkRegion.c render.c buffer.c \
    window.c navigation.c
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

# Debugging
debug: CFLAGS += -g -O0 -DDEBUG
debug: ae

# Header Dependencies
ae.o : keyPress.h minibuffer.h pointMarkRegion.h render.h \
       buffer.h window.h
keyPress.o : ae.h
minibuffer.o : ae.h keyPress.h
statusBar.o : ae.h
pointMarkRegion.o : minibuffer.h ae.h
render.o : ae.h statusBar.h pointMarkRegion.h buffer.h
buffer.o : ae.h minibuffer.h pointMarkRegion.h
window.o : ae.h
navigation.o : ae.h buffer.h window.h pointMarkRegion.h

# Targets
.phony: install tags

tags:
	find src/ -name "*.[ch]" -print | etags -

install: CFLAGS += -O2 -DNDEBUG
install: ae 
	mv ae ~/bin
