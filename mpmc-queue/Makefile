include ../benchmarks.mk

BENCH := mpmc-queue

BENCH_BINARY := $(BENCH).o

TESTS := main testcase1 testcase2

all: $(TESTS)
	../generate.sh $(notdir $(shell pwd))

%.o : %.cc
	$(CXX) -c -fPIC -MMD -MF .$@.d -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(TESTS): % : %.o  $(BENCH_BINARY)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

-include .*.d 

clean:
	rm -rf $(TESTS) *.o .*.d *.dSYM

.PHONY: clean all
