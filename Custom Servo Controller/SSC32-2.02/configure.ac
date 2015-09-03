#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(ssc-32-gcc, 2.02GP, ch@dot-heine.de)
AC_CONFIG_SRCDIR([src/abcd.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([tar-ustar gnu 1.10 check-news dist-bzip2])
AM_MAINTAINER_MODE

ac_tool_prefix=avr-
host_alias=avr
cross_compiling=yes

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_ARG_WITH([mcu],
  AC_HELP_STRING([--with-mcu=TARGET],
    [Use TARGET as destination AVR MCU. Default is ``atmega168'']),
  [case "$withval" in
	yes|no)
	  AC_MSG_ERROR([Need to specify the target MCU])
	  ;;
   	*)
	  MCU="$withval"
	  ;;
  esac],
  [MCU=atmega168])
AC_SUBST([MCU])

if test "${CFLAGS+set}" != set; then
  CFLAGS="-Os -ggdb3"
fi

case "$MCU" in
    atmega8)
#    CC_MCU_DEFS="-mtiny-stack"
    CC_MCU_DEFS=""
    LD_MCU_DEFS="-Wl,--pmem-wrap-around=8k -mmcu=$MCU"
    LDSCRIPT=ldscript.m8
    ;;
    atmega168)
    CC_MCU_DEFS="-mtiny-stack"
    LD_MCU_DEFS="-Wl,--pmem-wrap-around=16k -mmcu=$MCU"
    LDSCRIPT=ldscript.m168
    ;;
    *)
    ;;
esac
AC_SUBST([CC_MCU_DEFS])
AC_SUBST([LD_MCU_DEFS])
AC_SUBST([LDSCRIPT])

# Checks for programs.
AC_PROG_CC
AM_PROG_AS
AC_CHECK_TOOL([SIZE], [size])
AC_CHECK_TOOL([OBJCOPY], [objcopy])
AC_CHECK_PROGS([HEX2ABL], [hex2abl true])

AC_PATH_PROG([PERL], [perl])
AC_ARG_VAR([PERL], "The full path to the perl-interpreter on your machine.")

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.

AC_CONFIG_FILES([gccasm:gccasm.pl], [chmod +x gccasm])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
