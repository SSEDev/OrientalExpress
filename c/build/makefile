##################################################################
# Usage: make [all | clean | premake | BUILD_TYPE=[Debug/Release]]
#             [SOURCE_PATH=..][OS_TYPE=[Linux/Unix/Windows]]
#
#  make all                make all target : eps library
#                                            simple example
#                                            complex example
#  make clean              remove all target in dist directory
#  make premake            create dist directory
#  make BUILD_TYPE=Debug   compile debug version of target
#  make BUILD_TYPE=Release compile release version of target
#  make SOURCE_PATH=..     compile soure defined by SOURCE_PATH
##################################################################

SOURCE_PATH = ..

#operation system configuration
OS_TYPE = Linux

ifeq (${OS_TYPE}, Linux)
	MACRODEF = -D__LINUX__
	CFLAGS = -Wall -Werror -fPIC 
	MKDIR = mkdir -p 
	RMDIR = rm -rf 
	THREADFLAG = -lrt
	target_obj_path = $(SOURCE_PATH)/obj
	target_lib_path = $(SOURCE_PATH)/lib
	target_exe_path = $(SOURCE_PATH)/bin
endif

ifeq (${OS_TYPE}, Unix)
	MACRODEF = -D__HPUX__
	CFLAGS = -Wall -Werror -fPIC 
	MKDIR = mkdir -p 
	RMDIR = rm -rf 
	THREADFLAG = -lrt
	target_obj_path = $(SOURCE_PATH)/obj
	target_lib_path = $(SOURCE_PATH)/lib
	target_exe_path = $(SOURCE_PATH)/bin
endif

ifeq (${OS_TYPE}, Windows)
	MACRODEF = -D__WINDOWS__
	CFLAGS = -Wall -Werror
	MKDIR = mkdir
	RM = del
	RMDIR = rd /s/q 
	THREADFLAG = -lwsock32
	target_obj_path = $(SOURCE_PATH)\obj
	target_lib_path = $(SOURCE_PATH)\lib
	target_exe_path = $(SOURCE_PATH)\bin
endif

#build type configuration
BUILD_TYPE  = Debug

#compiler configuration
CC  = gcc

ifeq (${BUILD_TYPE}, Debug)
	CFLAGS += -g
endif

ifeq (${BUILD_TYPE}, Release)
	CFLAGS += -O1
endif

########################################
#main target: include all sub target
########################################
#all : libeps epsExample
all : libeps epsExample
.PHONY : all

########################################
#premake sub target
########################################
.PHONY : premake
premake :
	$(MKDIR) $(target_lib_path) $(target_exe_path) $(target_obj_path)

########################################
#epslib sub target
########################################
libeps_source_path = cmn eps step fast tcp udp
libeps_target   = $(target_lib_path)/libeps.a
libeps_sources  = $(wildcard $(foreach source_path,$(libeps_source_path),$(SOURCE_PATH)/src/$(source_path)/*.c))
libeps_includes = $(foreach source_path,$(libeps_source_path),-I$(SOURCE_PATH)/src/$(source_path)/)
libeps_objects  = $(foreach source_file,$(libeps_sources),$(patsubst %.c,%.o,$(source_file)))

$(libeps_objects):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $(target_obj_path)/$(notdir $@) $(libeps_includes) $(MACRODEF)
libeps : $(libeps_objects)
	ar -r $(libeps_target) $(target_obj_path)/*.o

########################################
##example sub target
########################################
epsExample : epsSimple epsComplex

EPSLIBFLAG  = -L$(target_lib_path) -leps

#simple example : udp
simple_soureces = $(SOURCE_PATH)/src/test/simpleEpsClient.c
simple_includes = $(libeps_includes)
epsSimple : libeps
	$(CC) $(CFLAGS) -o $(target_exe_path)/$@ $(simple_soureces) $(simple_includes) $(EPSLIBFLAG) $(THREADFLAG) $(MACRODEF)

#complex example : tcp/udp
complex_soureces = $(SOURCE_PATH)/src/test/complexEpsClient.c
complex_includes = $(libeps_includes)
epsComplex : libeps
	$(CC) $(CFLAGS) -o $(target_exe_path)/$@ $(complex_soureces) $(complex_includes) $(EPSLIBFLAG) $(THREADFLAG) $(MACRODEF)

#clean all binary
.PHONY : clean
clean :
	$(RMDIR) $(target_exe_path) $(target_lib_path) $(target_obj_path)
