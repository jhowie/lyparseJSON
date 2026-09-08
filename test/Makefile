# MacOS Makefile for lyparseJSON test utilities
#
ARCHITECTURE = $(shell uname -m)
CC=cc
CFLAGS=-g -I..
OBJDIR=../obj/$(ARCHITECTURE)/test
LIBDIR=../lib/$(ARCHITECTURE)
TESTOBJS=$(addprefix $(OBJDIR)/,testfile.o testmem.o testmem2.o, testcreate.o teststrings.o)
TESTS=testfile testmem testmem2 testcreate teststrings
LDFLAGS=-L$(LIBDIR) -llyparseJSON

all: $(TESTS)

#
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(OBJDIR)/$(<:.c=.o)

testfile: $(OBJDIR)/testfile.o
	$(CC) -o $(OBJDIR)/testfile $(OBJDIR)/testfile.o $(LDFLAGS)

testmem: $(OBJDIR)/testmem.o
	$(CC) -o $(OBJDIR)/testmem $(OBJDIR)/testmem.o $(LDFLAGS)

testmem2: $(OBJDIR)/testmem2.o
	$(CC) -o $(OBJDIR)/testmem2 $(OBJDIR)/testmem2.o $(LDFLAGS)

testcreate: $(OBJDIR)/testcreate.o
	$(CC) -o $(OBJDIR)/testcreate $(OBJDIR)/testcreate.o $(LDFLAGS)

teststrings: $(OBJDIR)/teststrings.o
	$(CC) -o $(OBJDIR)/teststrings $(OBJDIR)/teststrings.o $(LDFLAGS)

clean:
	rm -rf $(OBJDIR) 

$(TESTOBJS): $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
