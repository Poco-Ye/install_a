#########################################################################
# COMMON COMPILATION FLAGS                                              #
#########################################################################
TOOLCHAIN_PATH  ?= ~/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/

CROSS_COMPILER  ?= arm-linux-gnueabihf
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
