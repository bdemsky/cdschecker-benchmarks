include ../benchmarks.mk

NORMAL_TESTS := testcase1 testcase2

WILDCARD_TESTS := $(patsubst %, %_wildcard, $(NORMAL_TESTS))

TESTS := $(NORMAL_TESTS) $(WILDCARD_TESTS)

all: $(TESTS)

$(WILDCARD_TESTS): CXXFLAGS += -DWILDCARD

$(WILDCARD_TESTS): %_wildcard : %.cc hashmap_wildcard.h 
	$(CXX) -o $@ $< $(SPEC_OBJ) $(CXXFLAGS) $(LDFLAGS)

$(NORMAL_TESTS): % : %.cc hashmap.h
	$(CXX) -o $@ $< $(SPEC_OBJ) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *.d $(TESTS)
