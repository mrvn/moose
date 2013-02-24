# Moose makefile

ifndef srctree
# Top level needs to set srctree so other find us
srctree := $(if $(srctree),$(srctree),$(CURDIR))
export srctree
endif

# Set VERBOSE if you want to see the commands being executed
ifdef VERBOSE
  L = @:
  Q =
else
  L = @echo
  Q = @
  MAKEFLAGS += --no-print-directory
endif
export L Q

# Toplevel rule to call by default
all: built-in
	$(Q) $(MAKE) moose.img moose.symbols

include Makefile.options

clean: localclean
	$(Q) $(MAKE) -f $(srctree)/Makefile.built clean

localclean:
	$(Q) rm -f moose.img moose.elf moose.symbols

mrproper: localclean
	$(Q) $(MAKE) -f $(srctree)/Makefile.built mrproper

%.img: %.elf
	$(L) Objcopy $@
	$(Q) $(OBJCOPY) $< -O binary $@

%.symbols: %.elf
	$(L) Objdump $@
	$(Q) $(OBJDUMP) -t $< | sort | $(CXXFILT) >$@

built-in:
	$(Q) $(MAKE) -f $(srctree)/Makefile.built PARENT= built-in

%.elf: built-in.o link-arm-eabi.ld Makefile Makefile.options
	$(L) Linking $@
	$(Q) $(CXX) $(LDFLAGS) built-in.o -lgcc -Tlink-arm-eabi.ld -o $@

precious: moose.elf
