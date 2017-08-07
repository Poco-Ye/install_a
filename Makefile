# You can adjust the toolchain as you need
TOOLCHAIN_PATH ?= ~/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/
CROSS_COMPILER ?= arm-linux-gnueabihf
rm=/bin/rm -f
cp=/bin/cp -f
CC= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-g++
AR= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ar cr
RANLIB=$(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ranlib
STRIP=$(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-strip

REV=$(shell svnversion -c | awk -F":" '{ print $$2 }')

MARCH ?= armv7-a
INCS= -I./include -I./
SUBDIRS = MCPControl ion obfuse kcapi
CFLAGS =  -g -Os -march=$(MARCH) -Wall

OBJS= src/rtk_main.o src/rtk_burn.o src/rtk_urltar.o src/rtk_tar.o src/rtk_imgdesc.o src/rtk_fwdesc.o src/rtk_mtd.o src/rtk_common.o src/rtk_config.o src/rtk_factory.o src/rtk_parameter.o src/rtk_tagflow.o src/rtk_boottable.o src/rtk_customer.o

LIBS += -Wl,--start-group -L./lib/ -lMCP -lion -lpthread -lobfuse -lkcapi -Wl,--end-group

DEFINES += -DEMMC_SUPPORT
DEFINES += -DNFLASH_LAOUT
#DEFINES += -DGENERIC_LINUX
#DEFINES += -DUSE_SE_STORAGE
#DEFINES += -DNAS_ENABLE
#DEFINES += -DHYPERVISOR
#DEFINES += -DTEE_ENABLE
# 8MB SPI layout
#DEFINES += -DCONFIG_BOOT_FROM_SPI
CFLAGS += $(INCS) $(DEFINES)

all: subdir install

subdir:
	for i in $(SUBDIRS); do $(MAKE) -C $$i; done;			

install: $(OBJS) install_a.cpp
	$(shell echo "#define SVN_REV \"$(REV)\"" > include/svnver.h)
	$(CC) $(CFLAGS) -c install_a.cpp
	$(CC) -o install_a $(CFLAGS) $(OBJS) install_a.o $(LIBS) -static
	$(STRIP) install_a

clean:
	for i in $(SUBDIRS); do $(MAKE) clean -C $$i; done;	
	rm -rf $(OBJS) *.a *.o *.bak mapfile rtktest install_a oldup newup ./src/*.bak ./include/*.bak *.s *.ii customer

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
