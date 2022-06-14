ifneq ($(V), 1)
MFLAGS += --no-print-dir
Q := @
CC = gcc
endif

.PHONY: FORCE

all: libopencm3
	@$(Q)$(MAKE) $(MFLAGS) -C src $@

libopencm3: FORCE
	@$(Q)$(MAKE) $(MFLAGS) -C libopencm3 TARGETS=stm32/f1

init:
	$(Q)if [ ! -f libopencm3/Makefile ]; then \
		echo "Initialising git submodules..." ;\
		git submodule init ;\
		git submodule update ;\
	fi

help:
	$(Q)$(MAKE) $(MFLAGS) -C src $@

flash: libopencm3
	@$(Q)$(MAKE) $(MFLAGS) -C src $@

init_flash: libopencm3
	@$(Q)$(MAKE) $(MFLAGS) -C src $@

clean:
	$(Q)$(MAKE) $(MFLAGS) -C libopencm3 $@
	$(Q)$(MAKE) $(MFLAGS) -C src $@

