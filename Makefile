CC := cc
LD := cc
YACC := yacc

CFLAGS := -Wall -g -std=c99 -MMD

COMMON_OBJS := printer.o runtime.o util.o

FIC_OBJS := fi-parser.o fic.o $(COMMON_OBJS)

all: bootstrap1

.PHONY: clean
clean:
	rm -f *.[do] fic bootstrap1{,.c}

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%-parser.c: %-parser.y
	$(YACC) -o $@ $<

bootstrap1.c: bootpass1.fi bootmain1.fi fic
	cat bootpass1.fi bootmain1.fi | ./fic >$@

bootstrap1.o: bootstrap1.c
	$(CC) $(CFLAGS) -Wno-unused-but-set-variable -c $<

bootstrap1: bootstrap1.o hi-parser.o hic.o bootstrap1.o $(COMMON_OBJS)
	$(LD) -o $@ $^

fic: $(FIC_OBJS)
	$(LD) -o $@ $^

-include *.d
