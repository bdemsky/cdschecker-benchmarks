include ../benchmarks.mk

BENCH := stack

NORMAL_TESTS := testcase1

WILDCARD_TESTS := $(patsubst %, %_wildcard, $(NORMAL_TESTS))

TESTS := $(NORMAL_TESTS) $(WILDCARD_TESTS)

all: $(TESTS)

$(WILDCARD_TESTS): %_wildcard : %.cc $(BENCH)_wildcard.h 
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(NORMAL_TESTS): % : %.cc $(BENCH).h
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *.d $(TESTS)
