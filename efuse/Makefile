##############################################
# Tool Chain
##############################################
TOOLCHAIN_PATH = ~/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/

CROSS_COMPILER = arm-linux-gnueabihf
rm=/bin/rm -f
cp=/bin/cp -f
CC= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-g++
AR= $(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ar cr
RANLIB=$(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-ranlib
STRIP=$(TOOLCHAIN_PATH)/bin/$(CROSS_COMPILER)-strip

LIBNAME= ../lib/libefuse.a

OBJS = rtk_efuse.o

CFLAGS += $(DEFINES) $(MYDEFS)
CFLAGS += -I../include -I../

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
#	make -C widget
	$(rm) $@
	$(AR) $@ $(OBJS)
	$(RANLIB) $@

.c.o:
	$(rm) -f $@
	$(SHOW_COMMAND)echo -=--=--=- [$<] -=--=--=--=--=--=--=--=--=-
	$(SHOW_COMMAND)$(CC) $(CFLAGS) $(DEBUGFLAG) $(WARNING) -c $<

clean:
	$(rm) $(OBJS) $(LIBNAME) core *~

