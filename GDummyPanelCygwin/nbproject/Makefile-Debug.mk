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
CND_PLATFORM=Cygwin-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/83ecf3c1/Button.o \
	${OBJECTDIR}/_ext/83ecf3c1/ComplexWindow.o \
	${OBJECTDIR}/_ext/83ecf3c1/Fudge.o \
	${OBJECTDIR}/_ext/83ecf3c1/GDummyPanel.o \
	${OBJECTDIR}/_ext/83ecf3c1/GPIO.o \
	${OBJECTDIR}/_ext/83ecf3c1/GPanel.o \
	${OBJECTDIR}/_ext/83ecf3c1/GPanelObject.o \
	${OBJECTDIR}/_ext/83ecf3c1/NumBox.o \
	${OBJECTDIR}/_ext/83ecf3c1/StringEditWin.o \
	${OBJECTDIR}/_ext/83ecf3c1/WindowObject.o


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
LDLIBSOPTIONS=/cygdrive/C/cygwin64/lib/libncurses.a -lpanel

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanelcygwin.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanelcygwin.exe: /cygdrive/C/cygwin64/lib/libncurses.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanelcygwin.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gdummypanelcygwin ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/83ecf3c1/Button.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/Button.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/Button.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/Button.cpp

${OBJECTDIR}/_ext/83ecf3c1/ComplexWindow.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/ComplexWindow.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/ComplexWindow.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/ComplexWindow.cpp

${OBJECTDIR}/_ext/83ecf3c1/Fudge.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/Fudge.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/Fudge.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/Fudge.cpp

${OBJECTDIR}/_ext/83ecf3c1/GDummyPanel.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GDummyPanel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/GDummyPanel.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GDummyPanel.cpp

${OBJECTDIR}/_ext/83ecf3c1/GPIO.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPIO.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/GPIO.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPIO.cpp

${OBJECTDIR}/_ext/83ecf3c1/GPanel.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPanel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/GPanel.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPanel.cpp

${OBJECTDIR}/_ext/83ecf3c1/GPanelObject.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPanelObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/GPanelObject.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/GPanelObject.cpp

${OBJECTDIR}/_ext/83ecf3c1/NumBox.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/NumBox.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/NumBox.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/NumBox.cpp

${OBJECTDIR}/_ext/83ecf3c1/StringEditWin.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/StringEditWin.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/StringEditWin.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/StringEditWin.cpp

${OBJECTDIR}/_ext/83ecf3c1/WindowObject.o: /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/WindowObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/83ecf3c1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83ecf3c1/WindowObject.o /cygdrive/C/cygwin64/home/John-Bradley/GDummyPanel/WindowObject.cpp

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
