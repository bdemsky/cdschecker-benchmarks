include ../benchmarks.mk

TESTNAME = dekker-fences dekker-fences-wildcard1 dekker-fences-wildcard2

all: $(TESTNAME)

$(TESTNAME): % : %.cc
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TESTNAME) *.o
