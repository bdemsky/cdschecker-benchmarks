DIRS := register-acqrel register-relaxed ms-queue linuxrwlocks mcs-lock \
	chase-lev-deque-bugfix treiber-stack ticket-lock seqlock read-copy-update \
	concurrent-hashmap spsc-bugfix mpmc-queue barrier \
	chase-lev-deque-bugfix-loose ms-queue-loose blocking-mpmc-example

.PHONY: $(DIRS)

all: $(DIRS)

clean: $(DIRS:%=clean-%)

$(DIRS):
	$(MAKE) -C $@

clean-%:
	-$(MAKE) -C $* clean
