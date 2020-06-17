vpath %.c src
vpath %.h src

CLOC := $(shell command -v cloc 2> /dev/null)
TAGS := $(shell command -v etags 2> /dev/null)

# ####################################################################
#			     SOURCE CODE
# ####################################################################

# Source Code
SRC=ae.c keyPress.c minibuffer.c statusBar.c \
    pointMarkRegion.c render.c buffer.c \
    window.c navigation.c files.c state.c \
    edit.c
CFLAGS=-Wall -Wextra -pedantic -std=c99

# ####################################################################
#		      OBJECT FILES AND LIBRARIES
# ####################################################################

# Object Files
OBJS=$(SRC:.c=.o)

# Libraries
#LIBS=-lcurses -lreadline -lmenu
LIBS=-lcurses -lmenu

# ####################################################################
#			  BUILD DEPENDENCIES
# ####################################################################

# Header Dependencies
default: debug

.depend: $(SRC)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend

include .depend

# ####################################################################
#			  BUILD THE PROJECT
# ####################################################################

# Build Files
ae: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)
	@mkdir -p obj
	@mv *.o obj

%.o: %.c %.h ae.h
	$(CC) -o $@ -c $(CFLAGS) $<

# ####################################################################
#			   DEBUGGING FLAGS
# ####################################################################

# Debugging
debug: CFLAGS += -g -O0 -DDEBUG
debug: .depend ae tags stats

# Targets
.PHONY: install tags stats default debug

# ####################################################################
#			      STATS/TAGS
#
# These targets assume cloc and etags installed on local system.
#
# ####################################################################

tags:
	@echo "\nUpdating TAGS file..."
	@find src/ -name "*.[ch]" -print | etags -
	@ls -l TAGS
	@echo "\n"

stats:
ifdef CLOC
	@cloc .
else
	@wc src/*.c
endif

# ####################################################################
#			     INSTALLATION
#
# Install AndyEdit in bin/ directory of user's HOME.
#
# ####################################################################

install: CFLAGS += -O2 -DNDEBUG
install: ae .depend
	mv ae ~/bin
