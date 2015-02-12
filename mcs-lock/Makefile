include ../benchmarks.mk

TESTNAME = mcs-lock

WILDCARD_TESTS = testcase1

all: $(TESTNAME) $(WILDCARD_TESTS)
	echo $^

$(TESTNAME): $(TESTNAME).cc $(TESTNAME).h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

$(WILDCARD_TESTS): % : %.cc $(TESTNAME)-wildcard.h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TESTNAME) *.o $(WILDCARD_TESTS) 
