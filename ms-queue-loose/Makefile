include ../benchmarks.mk

BENCH := queue

BENCH_BINARY := $(BENCH).o

TESTS := main testcase1 testcase2 testcase3 testcase4

all: $(TESTS)
	../generate.sh $(notdir $(shell pwd))

%.o : %.c
	$(CC) -c -fPIC -MMD -MF .$@.d -o $@ $< $(CFLAGS) $(LDFLAGS)

$(TESTS): % : %.o $(BENCH_BINARY)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

-include .*.d 

clean:
	rm -rf $(TESTS) *.o .*.d *.dSYM

.PHONY: clean all
