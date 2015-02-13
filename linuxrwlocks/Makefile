include ../benchmarks.mk

TESTNAME = linuxrwlocks
WILDCARD_TESTS = testcase1 testcase2 testcase_full
all: $(TESTNAME) $(WILDCARD_TESTS)

$(TESTNAME): $(TESTNAME).c $(TESTNAME).h
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

$(WILDCARD_TESTS): % : %.c $(TESTNAME)-wildcard.h
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TESTNAME) *.o $(WILDCARD_TESTS)
