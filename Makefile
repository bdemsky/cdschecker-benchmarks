DIRS := ms-queue linuxrwlocks mcs-lock \
	chase-lev-deque-bugfix ticket-lock seqlock read-copy-update \
	concurrent-hashmap spsc-bugfix mpmc-queue

.PHONY: $(DIRS)

all: $(DIRS)

clean: $(DIRS:%=clean-%)

$(DIRS):
	$(MAKE) -C $@

clean-%:
	-$(MAKE) -C $* clean
