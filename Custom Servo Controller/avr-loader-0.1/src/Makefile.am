EXTRA_DIST = bootldr8.asm bootldr8.txt bootldr168.asm bootldr168.txt

INCLUDES = -I $(srcdir)/../headers/

CCASFLAGS :=
CCAS = @AVRA@

noinst_DATA = bootldr8.hex bootldr168.hex

SUFFIXES = .hex .obj .cof .asm

# Delete all the other stuff and only keep the intel hex-file; the
# boot-loaders do not access the EEPROM, so what.
%.hex: %.asm
	@AVRA@ $(INCLUDES) -o $@ $<
	-rm -f ${@:.hex=.cof} ${@:.hex=.obj} ${@:.hex=.eep.hex}

# Disassemble target
%.dis: %.hex
	@AVROBJDUMP@ --target ihex --disassemble-all -m avr:4 $< > $@

clean-local:
	-rm -f *.hex *.cof *.obj

# Burn the fuses for 8MHz internal oscillator
#fuse168: bootldr168.hex
#	@AVRDUDE@ @AVRDUDEOPTS@ \
# -U efuse:w:0xFE:m -U hfuse:w:0xDC:m -U lfuse:w:0xE2:m
