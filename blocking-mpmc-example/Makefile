include ../benchmarks.mk

BENCH := queue

BENCH_BINARY := $(BENCH).o

TESTS := main testcase1 testcase2 testcase3 testcase4

all: $(TESTS)
	../generate.sh $(notdir $(shell pwd))

%.o : %.c
	$(CXX) -c -fPIC -MMD -MF .$@.d -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(TESTS): % : %.o $(BENCH_BINARY)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

-include .*.d 

clean:
	rm -rf $(TESTS) *.o .*.d *.dSYM

.PHONY: clean all
