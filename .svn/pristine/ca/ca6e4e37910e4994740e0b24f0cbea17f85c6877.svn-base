#########################################################################
# COMMON COMPILATION FLAGS                                              #
#########################################################################
TOOLCHAIN_PATH  ?= ~/toolchain/asdk-4.8.1-a7-EL-3.10-0.9.33-a32nt-130828/

CROSS_COMPILER  ?= arm-linux
CC		:= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-g++
AR		:= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ar cr
RANLIB		:= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ranlib
STRIP		:= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-strip

CFLAGS          := -Wall
CFLAGS          += -c -fPIC

DEBUG       	?= 0
ifeq ($(DEBUG), 1)
CFLAGS          += -DDEBUG -O0 -g
endif

RM		:= /bin/rm -rf
CP		:= /bin/cp -f
