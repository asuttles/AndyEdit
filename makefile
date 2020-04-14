SRCDIR=src
SOURCES=ae.c
DEPS=ae.h
CFLAGS=-Wall -Wextra -pedantic -std=c99

OBJDIR=obj
OBJS=ae.o

LIBS=-lcurses -lreadline


$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/$(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

ae: $(OBJDIR)/$(OBJS)
	$(CC) -o $@ $^ $(LIBS)

.phony: clean install

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*~

install: ae clean
	mv ae ~/bin
