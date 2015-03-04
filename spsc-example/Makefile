include ../benchmarks.mk

TESTNAME = spsc-queue

all: $(TESTNAME)

$(TESTNAME): $(TESTNAME).cc queue.h
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TESTNAME) *.o
