include ../benchmarks.mk

TESTS := table table-normal

all: $(TESTS)

table: main.cc
	$(CXX) -o $@ $^ $(SPEC_OBJ) $(CXXFLAGS) -std=c++0x $(LDFLAGS)

table-normal: main.cc
	$(CXX) -o $@ $^ -std=c++0x -DNORMAL

clean:
	rm -f *.o *.d $(TESTS)
