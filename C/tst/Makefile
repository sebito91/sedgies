CFLAGS=-std=c11 -pedantic -Wall -Werror -ggdb
CC=gcc
OPT_CFLAGS=$(CFLAGS) -O3 -fomit-frame-pointer
LIBS=-lcunit

SUPPORTFILES=trie.h trie.c

TESTFILES=trie_test.c $(SUPPORTFILES)

TESTS=trie_test.c

.PHONY: all
all: trie

test_trie: trie_test
	./trie_test

# Use this target to run all the unit tests
.PHONY: test
test: $(patsubst %.c,%,$(TESTS))
	for t in $^; do ./$$t ; done

%_test: %_test.c $(TESTFILES)
	gcc -o $@ $(OPT_CFLAGS) $(filter %.c,$^) $(LIBS)

trie: $(SUPPORTFILES)
	gcc -o $@ $(CFLAGS) $(filter %.c,$^) $(LIBS)

.PHONY: clean
clean:
	-rm -rf $(patsubst %.c,%,$(TESTS)) trie trie.o tester tester.o random random.o

###############################################################
# The rest of this file is for internal use; please ignore
##############################################################

%.o: %.c
	$(CC) $(OPT_CFLAGS) $(LIBS) -c -o $@ $<

%: %.o
	$(CC) $(OPT_CFLAGS) $(LIBS) -o $@ $^

tester: tester.o

tester.o: tester.c

random: random.o

random.o: random.c
