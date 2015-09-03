#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(avr-loader, 0.1, ch@dot-heine.de)
AC_CONFIG_SRCDIR([src/bootldr168.asm])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([tar-pax gnu 1.10 check-news dist-bzip2])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX

AC_PATH_PROG([AVRA], [avra])
AC_ARG_VAR([AVRA],
  [Set to the full path of ``avra'', the "Advanced AVR macro Assembler"])

AC_PATH_PROG([AVROBJDUMP], [avr-objdump])
AC_ARG_VAR([AVROBJDUMP],
  [Set to the full path of the GNU avr-objdump utility])

AC_PATH_PROG([AVROBJCOPY], [avr-objcopy])
AC_ARG_VAR([AVROBJDUMP],
  [Set to the full path of the GNU avr-objcopy utility])

dnl AC_PATH_PROG([AVRDUDE], [avrdude])
dnl AC_ARG_VAR([AVRDUDE],
dnl   [Set to the full path of the avrdude flash program])

dnl AC_ARG_WITH([avrdude-options],
dnl   AC_HELP_STRING([--with-avrdude-options=OPTSTR],
dnl     [Use OPTSTR to specify options for avrdude. The part-number (MCU-type) will
dnl     be chosen automatically depending on the boot-loader to flash.
dnl     Default: "-B 4 -c dragon_isp -P usb -n"]),
dnl   [case "$withval" in
dnl       yes|no)
dnl          AC_MSG_ERROR([Need to specify the option-string literally])
dnl          ;;
dnl       *)
dnl          AVRDUDEOTPS="$withval"
dnl 	 ;;
dnl    esac],
dnl    [AVRDUDEOPTS="-B 4 -c dragon_isp -P usb -n"])
dnl AC_ARG_VAR([AVRDUDEOPTS],
dnl   [Set to the full set of options to pass to avrdude, excluding the chip-type])

AC_CONFIG_FILES([Makefile src/Makefile util/Makefile])
AC_OUTPUT
