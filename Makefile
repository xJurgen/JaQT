CC = gcc
Q := @
MFLAGS += --no-print-dir

all: bootloader copy_bootloader app copy_app merge_tool_compile merge

merge_tool_compile:
	$(Q)$(CC) fw/bootloader/sketch_combiner/main.c -o fw/bootloader/sketch_combiner/merge_binaries.bin

bootloader:
	$(Q)$(MAKE) $(MFLAGS) -C fw/bootloader generic-pc13
copy_bootloader:
	-$(Q)mkdir -p build
	-$(Q) cp fw/bootloader/bootloader_only_binaries/bootloader.bin build/

app:
	$(Q)$(MAKE) $(MFLAGS) -C fw/JaQT2
	$(Q)echo
	$(Q)echo "JaQT2 Application done"
	$(Q)echo

copy_app:
	-$(Q)mkdir -p build
	-$(Q)cp fw/JaQT2/src/jaqt2.bin build/

merge:
	-$(Q) ./fw/bootloader/sketch_combiner/merge_binaries.bin build/bootloader.bin build/jaqt2.bin build/bootloader_jaqt2.bin
	$(Q)echo "Merged"

clean:
	-$(Q)$(MAKE) $(MFLAGS) -C fw/JaQT2/libopencm3 $@
	-$(Q)$(MAKE) $(MFLAGS) -C fw/JaQT2/src $@
	-$(Q)$(MAKE) $(MFLAGS) -C fw/bootloader clean-binaries
	-$(Q)rm -rf build/
	$(Q)echo
	$(Q)echo "Done"
	$(Q)echo

init_flash: all
	st-flash --reset write build/bootloader_jaqt2.bin  0x8000000

flash:
	-$(Q)$(MAKE) $(MFLAGS) -C fw/JaQT2 $@
