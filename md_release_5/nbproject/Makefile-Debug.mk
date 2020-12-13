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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/BinFileReader.o \
	${OBJECTDIR}/CombinedSummary.o \
	${OBJECTDIR}/DM7520/lib/librtd-dm75xx.o \
	${OBJECTDIR}/DMAReader.o \
	${OBJECTDIR}/DM_7520.o \
	${OBJECTDIR}/Filters.o \
	${OBJECTDIR}/IReader.o \
	${OBJECTDIR}/ISummary.o \
	${OBJECTDIR}/Loggers.o \
	${OBJECTDIR}/Parsers.o \
	${OBJECTDIR}/Recepts.o \
	${OBJECTDIR}/SummFileReader.o \
	${OBJECTDIR}/Summary.o \
	${OBJECTDIR}/UdpReader.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=gnu++0x `pkg-config libxml++-2.6 --cflags` `pkg-config --libs --cflags libmodbus` 
CXXFLAGS=-std=gnu++0x `pkg-config libxml++-2.6 --cflags` `pkg-config --libs --cflags libmodbus` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs libxml++-2.6` -lpthread  `pkg-config --libs libmodbus`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/md_release_5

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/md_release_5: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/md_release_5 ${OBJECTFILES} ${LDLIBSOPTIONS} -lrt

${OBJECTDIR}/BinFileReader.o: BinFileReader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BinFileReader.o BinFileReader.cpp

${OBJECTDIR}/CombinedSummary.o: CombinedSummary.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CombinedSummary.o CombinedSummary.cpp

${OBJECTDIR}/DM7520/lib/librtd-dm75xx.o: DM7520/lib/librtd-dm75xx.cpp
	${MKDIR} -p ${OBJECTDIR}/DM7520/lib
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DM7520/lib/librtd-dm75xx.o DM7520/lib/librtd-dm75xx.cpp

${OBJECTDIR}/DMAReader.o: DMAReader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DMAReader.o DMAReader.cpp

${OBJECTDIR}/DM_7520.o: DM_7520.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DM_7520.o DM_7520.cpp

${OBJECTDIR}/Filters.o: Filters.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Filters.o Filters.cpp

${OBJECTDIR}/IReader.o: IReader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/IReader.o IReader.cpp

${OBJECTDIR}/ISummary.o: ISummary.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ISummary.o ISummary.cpp

${OBJECTDIR}/Loggers.o: Loggers.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Loggers.o Loggers.cpp

${OBJECTDIR}/Parsers.o: Parsers.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Parsers.o Parsers.cpp

${OBJECTDIR}/Recepts.o: Recepts.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Recepts.o Recepts.cpp

${OBJECTDIR}/SummFileReader.o: SummFileReader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SummFileReader.o SummFileReader.cpp

${OBJECTDIR}/Summary.o: Summary.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Summary.o Summary.cpp

${OBJECTDIR}/UdpReader.o: UdpReader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UdpReader.o UdpReader.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -IDM7520/include `pkg-config --cflags libxml++-2.6` `pkg-config --cflags libmodbus`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
