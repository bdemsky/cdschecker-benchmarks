include ../benchmarks.mk

WILDCARD_TESTS = testcase1

all: seqlock $(WILDCARD_TESTS)

seqlock: seqlock.c seqlock.h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(WILDCARD_TESTS): % : %.c seqlock-wildcard.h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)
	
clean:
	rm -f seqlock $(WILDCARD_TESTS)
