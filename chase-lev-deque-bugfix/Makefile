include ../benchmarks.mk

BENCH := deque

NORMAL_TESTS := testcase1 testcase2 testcase3 testcase4 testcase5 testcase6 testcase7 testcase8 testcase9 testcase10

WILDCARD_TESTS := $(patsubst %, %_wildcard, $(NORMAL_TESTS))

TESTS := $(NORMAL_TESTS) $(WILDCARD_TESTS)

all: $(TESTS)

$(BENCH).o : $(BENCH).c $(BENCH).h
	$(CC) -o $@ $< $(CFLAGS) -c $(LDFLAGS)

$(BENCH)_wildcard.o : $(BENCH)_wildcard.c $(BENCH).h
	$(CC) -o $@ $< $(CFLAGS) -c $(LDFLAGS)

$(WILDCARD_TESTS): %_wildcard : %.c $(BENCH)_wildcard.o 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(NORMAL_TESTS): % : %.c $(BENCH).o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *.d $(TESTS)
