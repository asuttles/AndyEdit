vpath %.c src
vpath %.h src

# Source Code
SRC=ae.c keyPress.c minibuffer.c statusBar.c \
    pointMarkRegion.c render.c buffer.c \
    window.c navigation.c files.c state.c
CFLAGS=-Wall -Wextra -pedantic -std=c99

# Object Files
OBJS=$(SRC:.c=.o)

# Libraries
LIBS=-lcurses -lreadline -lmenu

# Build Files
ae: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)
	mv *.o obj

%.o: %.c %.h ae.h
	$(CC) -o $@ -c $(CFLAGS) $<

# Debugging
debug: CFLAGS += -g -O0 -DDEBUG
debug: ae tags

# Header Dependencies
ae.o : keyPress.h minibuffer.h pointMarkRegion.h render.h \
       buffer.h window.h navigation.h files.h state.h \
       state.h
keyPress.o : ae.h window.h
minibuffer.o : ae.h keyPress.h window.h files.h
statusBar.o : window.h
pointMarkRegion.o : minibuffer.h ae.h buffer.h state.h
render.o : ae.h statusBar.h pointMarkRegion.h buffer.h
buffer.o : ae.h minibuffer.h pointMarkRegion.h files.h state.h
window.o : ae.h
navigation.o : ae.h buffer.h window.h pointMarkRegion.h minibuffer.h
files.o : ae.h keyPress.h buffer.h minibuffer.h files.h
state.o : ae.h pointMarkRegion.h buffer.h minibuffer.h

# Targets
.phony: install tags

tags:
	@echo "\nUpdating TAGS file..."
	find src/ -name "*.[ch]" -print | etags -
	@echo "\n"
	@ls -l TAGS

install: CFLAGS += -O2 -DNDEBUG
install: ae 
	mv ae ~/bin
