#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/libraries/Onyx/Onyx.o \
	${OBJECTDIR}/libraries/SSC32/SSC32.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/HardwareSerial.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/Print.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/WMath.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/WString.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/i2c.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/main.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/sysfs.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_analog.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_digital.o \
	${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_shift.o \
	${OBJECTDIR}/libraries/libarduino/libraries/LiquidCrystal/LiquidCrystal.o \
	${OBJECTDIR}/libraries/libarduino/libraries/SPI/SPI.o \
	${OBJECTDIR}/libraries/libarduino/libraries/Stepper/Stepper.o \
	${OBJECTDIR}/libraries/libarduino/libraries/Wire/Wire.o \
	${OBJECTDIR}/libraries/libarduino/variants/beaglebone/variant.o \
	${OBJECTDIR}/libraries/libarduino/variants/cfa10036/variants.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/onyx-bbb

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/onyx-bbb: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/onyx-bbb ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/libraries/Onyx/Onyx.o: libraries/Onyx/Onyx.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/Onyx
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/Onyx/Onyx.o libraries/Onyx/Onyx.cpp

${OBJECTDIR}/libraries/SSC32/SSC32.o: libraries/SSC32/SSC32.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/SSC32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/SSC32/SSC32.o libraries/SSC32/SSC32.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/HardwareSerial.o: libraries/libarduino/cores/virtual/HardwareSerial.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/HardwareSerial.o libraries/libarduino/cores/virtual/HardwareSerial.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/Print.o: libraries/libarduino/cores/virtual/Print.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/Print.o libraries/libarduino/cores/virtual/Print.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/WMath.o: libraries/libarduino/cores/virtual/WMath.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/WMath.o libraries/libarduino/cores/virtual/WMath.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/WString.o: libraries/libarduino/cores/virtual/WString.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/WString.o libraries/libarduino/cores/virtual/WString.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/i2c.o: libraries/libarduino/cores/virtual/i2c.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/i2c.o libraries/libarduino/cores/virtual/i2c.c

${OBJECTDIR}/libraries/libarduino/cores/virtual/main.o: libraries/libarduino/cores/virtual/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/main.o libraries/libarduino/cores/virtual/main.cpp

${OBJECTDIR}/libraries/libarduino/cores/virtual/sysfs.o: libraries/libarduino/cores/virtual/sysfs.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/sysfs.o libraries/libarduino/cores/virtual/sysfs.c

${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring.o: libraries/libarduino/cores/virtual/wiring.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring.o libraries/libarduino/cores/virtual/wiring.c

${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_analog.o: libraries/libarduino/cores/virtual/wiring_analog.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_analog.o libraries/libarduino/cores/virtual/wiring_analog.c

${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_digital.o: libraries/libarduino/cores/virtual/wiring_digital.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_digital.o libraries/libarduino/cores/virtual/wiring_digital.c

${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_shift.o: libraries/libarduino/cores/virtual/wiring_shift.c 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/cores/virtual
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/cores/virtual/wiring_shift.o libraries/libarduino/cores/virtual/wiring_shift.c

${OBJECTDIR}/libraries/libarduino/libraries/LiquidCrystal/LiquidCrystal.o: libraries/libarduino/libraries/LiquidCrystal/LiquidCrystal.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/libraries/LiquidCrystal
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/libraries/LiquidCrystal/LiquidCrystal.o libraries/libarduino/libraries/LiquidCrystal/LiquidCrystal.cpp

${OBJECTDIR}/libraries/libarduino/libraries/SPI/SPI.o: libraries/libarduino/libraries/SPI/SPI.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/libraries/SPI
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/libraries/SPI/SPI.o libraries/libarduino/libraries/SPI/SPI.cpp

${OBJECTDIR}/libraries/libarduino/libraries/Stepper/Stepper.o: libraries/libarduino/libraries/Stepper/Stepper.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/libraries/Stepper
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/libraries/Stepper/Stepper.o libraries/libarduino/libraries/Stepper/Stepper.cpp

${OBJECTDIR}/libraries/libarduino/libraries/Wire/Wire.o: libraries/libarduino/libraries/Wire/Wire.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/libraries/Wire
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/libraries/Wire/Wire.o libraries/libarduino/libraries/Wire/Wire.cpp

${OBJECTDIR}/libraries/libarduino/variants/beaglebone/variant.o: libraries/libarduino/variants/beaglebone/variant.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/variants/beaglebone
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/variants/beaglebone/variant.o libraries/libarduino/variants/beaglebone/variant.cpp

${OBJECTDIR}/libraries/libarduino/variants/cfa10036/variants.o: libraries/libarduino/variants/cfa10036/variants.cpp 
	${MKDIR} -p ${OBJECTDIR}/libraries/libarduino/variants/cfa10036
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libraries/libarduino/variants/cfa10036/variants.o libraries/libarduino/variants/cfa10036/variants.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/onyx-bbb

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
