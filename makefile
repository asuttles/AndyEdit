vpath %.c src
vpath %.h src

# Source Code
SRC=ae.c keyPress.c minibuffer.c statusBar.c \
    pointMarkRegion.c render.c buffer.c \
    window.c navigation.c files.c state.c \
    edit.c
CFLAGS=-Wall -Wextra -pedantic -std=c99

# Object Files
OBJS=$(SRC:.c=.o)

# OS Type
OS_TYPE := $(shell uname -s)

# Libraries
LIBS=-lcurses -lreadline -lmenu

# Header Dependencies
default: .depend debug

ifeq ($OS_TYPE,Linux)
.depend: $(SRC)
	echo "Linux System"
	if [ -e ./.depend]; then
	   rm -f ./.depend
	fi
	$(CC) $(CFLAGS) -MM $^>>./.depend
else
	gcc -MM $(SRC) >>./.depend
endif

include .depend


# Build Files
ae: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)
	mv *.o obj

%.o: %.c %.h ae.h
	$(CC) -o $@ -c $(CFLAGS) $<

# Debugging
debug: CFLAGS += -g -O0 -DDEBUG
debug: ae tags .depend stats

# Targets
.phony: install tags stats

tags:
	@echo "\nUpdating TAGS file..."
	find src/ -name "*.[ch]" -print | etags -
	@echo "\n"
	@ls -l TAGS

stats:
	cloc src/*.[ch]

install: CFLAGS += -O2 -DNDEBUG
install: ae .depend
	mv ae ~/bin
