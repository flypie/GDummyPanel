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
	${OBJECTDIR}/_ext/5c0/Button.o \
	${OBJECTDIR}/_ext/5c0/ComplexWindow.o \
	${OBJECTDIR}/_ext/5c0/Fudge.o \
	${OBJECTDIR}/_ext/5c0/GDummyPanel.o \
	${OBJECTDIR}/_ext/5c0/GPIO.o \
	${OBJECTDIR}/_ext/5c0/GPanel.o \
	${OBJECTDIR}/_ext/5c0/GPanelObject.o \
	${OBJECTDIR}/_ext/5c0/NumBox.o \
	${OBJECTDIR}/_ext/5c0/StringEditWin.o \
	${OBJECTDIR}/_ext/5c0/WindowObject.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=gnu++14
CXXFLAGS=-std=gnu++14

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread-2.23 -lpanel -lncurses

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanellinux

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanellinux: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanellinux ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/5c0/Button.o: ../Button.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/Button.o ../Button.cpp

${OBJECTDIR}/_ext/5c0/ComplexWindow.o: ../ComplexWindow.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/ComplexWindow.o ../ComplexWindow.cpp

${OBJECTDIR}/_ext/5c0/Fudge.o: ../Fudge.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/Fudge.o ../Fudge.cpp

${OBJECTDIR}/_ext/5c0/GDummyPanel.o: ../GDummyPanel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/GDummyPanel.o ../GDummyPanel.cpp

${OBJECTDIR}/_ext/5c0/GPIO.o: ../GPIO.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/GPIO.o ../GPIO.cpp

${OBJECTDIR}/_ext/5c0/GPanel.o: ../GPanel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/GPanel.o ../GPanel.cpp

${OBJECTDIR}/_ext/5c0/GPanelObject.o: ../GPanelObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/GPanelObject.o ../GPanelObject.cpp

${OBJECTDIR}/_ext/5c0/NumBox.o: ../NumBox.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/NumBox.o ../NumBox.cpp

${OBJECTDIR}/_ext/5c0/StringEditWin.o: ../StringEditWin.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/StringEditWin.o ../StringEditWin.cpp

${OBJECTDIR}/_ext/5c0/WindowObject.o: ../WindowObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/WindowObject.o ../WindowObject.cpp

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
