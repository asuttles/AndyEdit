SRCDIR=src
SOURCES=ae.c

CFLAGS=-Wall -Wextra -pedantic -std=c99

LIBS=-lcurses -lreadline

ae: $(SRCDIR)/$(SOURCES)
	$(CC) $(SRCDIR)/$(SOURCES) -o ae $(CFLAGS) $(LIBS)
