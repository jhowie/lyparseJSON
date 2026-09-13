# MacOS Makefile for lyparseJSON
#
ARCHITECTURE = $(shell uname -m)
CC=cc
CFLAGS=
LEX=flex
YACC=bison
YACCFLAGS=-d -p parseJSON
LEXFLAGS=-P parseJSON --header-file=parseJSON.lex.h
OBJDIR=obj/$(ARCHITECTURE)
LIBDIR=lib/$(ARCHITECTURE)
LIBOBJS=$(addprefix $(OBJDIR)/,parseJSON.tab.o parseJSON.lex.o parseJSON_support.o)
INSTALLLIBDIR=/usr/local/lib
INSTALLINCLUDEDIR=/usr/local/include
#

%.c: %.y
%.c: %.l

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(OBJDIR)/$(<:.c=.o)

$(LIBDIR)/lyparseJSON.a: $(LIBOBJS) $(LIBDIR)
	ar -r $(LIBDIR)/liblyparseJSON.a $(LIBOBJS)
	ranlib $(LIBDIR)/liblyparseJSON.a

clean:
	rm -rf $(OBJDIR) $(LIBDIR) obj lib parseJSON.tab.[ch] parseJSON.lex.[ch]

install: $(LIBDIR)/liblyparseJSON.a parseJSON.h
	install -d -o root -g wheel $(INSTALLLIBDIR)
	install -c -o root -g wheel -m 0444 $(LIBDIR)/liblyparseJSON.a $(INSTALLLIBDIR)
	install -d -o root -g wheel $(INSTALLINCLUDEDIR)
	install -c -o root -g wheel -m 0444 parseJSON.h $(INSTALLINCLUDEDIR)

parseJSON.tab.c: parseJSON.y parseJSON.h parseJSON_support.h 
	$(YACC) $(YACCFLAGS) parseJSON.y

parseJSON.tab.h: parseJSON.y
	$(YACC) $(YACCFLAGS) parseJSON.y

parseJSON.lex.h: parseJSON.l

parseJSON.lex.c: parseJSON.l parseJSON.h parseJSON_support.h parseJSON.tab.h parseJSON.lex.h
	$(LEX) $(LEXFLAGS) parseJSON.l

all: $(LIBDIR)/liblyparseJSON.a

$(LIBOBJS): $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)