include ../benchmarks.mk

TESTS := table table_wildcard1

all: $(TESTS)

table: main.cc hashmap.h
	$(CXX) -o $@ $^ $(SPEC_OBJ) $(CXXFLAGS) -std=c++0x $(LDFLAGS)

table_wildcard1: main_wildcard1.cc hashmap_wildcard.h
	$(CXX) -o $@ $^ $(SPEC_OBJ) $(CXXFLAGS) -std=c++0x $(LDFLAGS)

table_wildcard2: main_wildcard1.cc hashmap_wildcard.h
	$(CXX) -o $@ $^ $(SPEC_OBJ) $(CXXFLAGS) -std=c++0x $(LDFLAGS)

clean:
	rm -f *.o *.d $(TESTS)
