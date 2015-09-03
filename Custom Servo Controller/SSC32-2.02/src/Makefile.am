DEFS = @DEFS@ -mmcu=@MCU@ @CC_MCU_DEFS@

EXTRA_DIST = \
 pulsedge.c serial.c \
 pulsedge.gccasm.c serial.gccasm.c \
 ldscript.m168 \
 ssc32.prj
BUILT_SOURCES = pulsedge.gccasm.c serial.gccasm.c

LDSCRIPT = @LDSCRIPT@

RELAX = -Wl,--relax

AM_CPPFLAGS = -DSWVER=\"@VERSION@-GCC\"

noinst_PROGRAMS = ssc32
noinst_DATA = ssc32.hex ssc32.abl ssc32.eep

ssc32_SOURCES = ssc32.c\
 compiler.h\
 abcd.c abcd.h\
 appdefs.h\
 cmdproc.c cmdproc.h\
 eeprom.c eeprom.h\
 globals.c globals.h\
 gpseq.c gpseq.h\
 hexapod.c hexapod.h\
 numtypes.h\
 pulsedge.gccasm.c pulsedge.h\
 regrw.c regrw.h\
 serial.gccasm.c serial.h

ssc32_LDFLAGS = $(RELAX) \
 @LD_MCU_DEFS@ -T $(srcdir)/$(LDSCRIPT) -Wl,-Map,ssc32.map

SUFFIXES = .gccasm.c .s .hex

clean-local:
	rm -f *.s *.map *.abl *.hex *.eep

distclean-local:
	rm -f *.gccasm.c 

%.gccasm.c : %.c
	${top_builddir}/gccasm $< > $@

%.s : %.c
	$(COMPILE) -S -o $@ $<

%.hex : %
	$(OBJCOPY) -O ihex -j .text $< $@
	$(SIZE) --format=sysv --radix=16  ssc32|head -n 6

%.eep : %
	$(OBJCOPY) -O ihex -j .eeprom $< $@.tmp
	tail -n +2 $@.tmp > $@
	rm -f $@.tmp

%.abl : %.hex
#	$(HEX2ABL) --page-size 64 $< $@
	$(HEX2ABL) --page-size 128 $< $@.tmp;\
 ( echo "SSC32-@VERSION@-GCC" ;\
 echo "................................................................";\
 cat $@.tmp ) > $@;\
 rm -f $@.tmp
