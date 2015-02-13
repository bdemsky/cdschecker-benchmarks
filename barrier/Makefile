include ../benchmarks.mk

TESTNAME = barrier

WILDCARD_TESTS = testcase

all: $(TESTNAME) $(WILDCARD_TESTS)

$(TESTNAME): $(TESTNAME).cc $(TESTNAME).h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(WILDCARD_TESTS): % : %.cc $(TESTNAME)-wildcard.h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TESTNAME) *.o $(WILDCARD_TESTS)
