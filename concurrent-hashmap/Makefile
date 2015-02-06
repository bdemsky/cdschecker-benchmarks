include ../benchmarks.mk

TESTS := table 

all: $(TESTS)

table: main.cc hashmap.h
	$(CXX) -o $@ $^ $(SPEC_OBJ) $(CXXFLAGS) -std=c++0x $(LDFLAGS)

clean:
	rm -f *.o *.d $(TESTS)
