#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <rtk_mtd.h>
//#include <rtk_imgdesc.h>
#include <rtk_fwdesc.h>
#include <rtk_def.h>
#include <rtk_parameter.h>
#include <rtk_common.h>
#include <rtk_def.h>
#include <errno.h>

static char MTD_CHAR_DEV_PATH[][20]={
#ifdef PC_SIMULATE
   "tmp/mtd0",
#endif
   "/dev/mtd/disc",
   "/dev/mtd/mtddisc",
   "/dev/mtddisc",
   "/dev/mtd0",
   "/dev/mtd/mtd0",
   "/dev/mtd0ro",
   "/dev/mtd/0"
};
static char MTD_BLOCK_DEV_PATH[][50]={
#ifdef PC_SIMULATE
   "tmp/mmcblk0",
   "tmp/mtdblock0",
#endif
   "/dev/mmcblk0",
   "/dev/mmcblk1",
   "/dev/block/mmcblk0",
   "/dev/block/mmcblk1",
   "/dev/block/mtdblockdisc",
   "/dev/mtdblockdisc",
   "/dev/mtdblock/mtdblockdisc",
   "/dev/mtdblock/disc",
   "/dev/mtdblock0",
   "/dev/block/mtdblock0",
   "/dev/mtdblock/0",
   "/dev/mtdblock/mtdblock0"
};
#ifdef __OFFLINE_GENERATE_BIN__
static char MTD_PSEUDO_DEV_PATH[] = {DEFAULT_TEMP_OUTPUT};
#endif

static char* pblock_name = NULL;

#ifndef __OFFLINE_GENERATE_BIN__
#if defined(NAS_ENABLE) && !defined(PC_SIMULATE)
int rtk_open_last_mtd(const char* prefix, char** mtd_name){
   char *name;
   int last_fd = -1, fd, i;

   name = (char*)malloc(50);
   memset(name, 0, 50);

   for(i = 0; ; i++){
       sprintf(name, "%s%d", prefix, i);
       /* Test with read-only access */
       fd = open(name, O_RDONLY);
       if(-1 == fd) break;
       if(last_fd >= 0){
          close(last_fd);
       }
       last_fd = fd;
   }
   if(last_fd >= 0){
      sprintf(name, "%s%d", prefix, i-1);
      *mtd_name = name;

      fd = open(name, O_RDWR);
      close(last_fd);
      last_fd = fd;
   }
   return last_fd;
}
#endif
int rtk_open_mtd_char(char** ppstr)
{
   int i;
   int dev_fd;
#if defined(NAS_ENABLE) && !defined(PC_SIMULATE)
   char *mtd_name;
   dev_fd = rtk_open_last_mtd("/dev/mtd", &mtd_name);
   if(dev_fd >= 0){
       install_info("[NAS]Open MTD_CHAR %s\r\n", mtd_name);
       if(NULL != ppstr) *ppstr = mtd_name;
       return dev_fd;
   }
#endif
   for(i = 0; i < (int)(sizeof(MTD_CHAR_DEV_PATH)/sizeof(MTD_CHAR_DEV_PATH[0])); i++)
   {
      dev_fd = open(MTD_CHAR_DEV_PATH[i], O_RDWR);
      if(-1!=dev_fd) break;
   }
   if(dev_fd < 0)
   {
      install_debug("open mtd char fail\r\n");
      return -1;
   }
   install_info("Open MTD_CHAR %s\r\n", MTD_CHAR_DEV_PATH[i]);
   if(NULL != ppstr) *ppstr = MTD_CHAR_DEV_PATH[i];
   return dev_fd;
}

int rtk_open_mtd_block(char** ppstr)
{
	int i;
	int dev_fd;

#if defined(NAS_ENABLE) && !defined(PC_SIMULATE)
   char *mtd_name;
   dev_fd = rtk_open_last_mtd("/dev/mtdblock", &mtd_name);
   if(dev_fd >= 0){
       install_info("[NAS]Open MTD_BLOCK %s\r\n", mtd_name);
       if(NULL != ppstr) *ppstr = mtd_name;
       pblock_name = mtd_name;
       return dev_fd;
   }
#endif

	i=0;
	do {
		for(; i < (int)(sizeof(MTD_BLOCK_DEV_PATH)/sizeof(MTD_BLOCK_DEV_PATH[0])); i++)
		{
			dev_fd = open(MTD_BLOCK_DEV_PATH[i], O_RDWR|O_SYNC);
			if(-1!=dev_fd) break;
		}
		if(dev_fd < 0)
		{
			install_debug("open mtd block fail\r\n");
			return -1;
		}
		install_info("Open MTD_BLOCK %s\r\n", MTD_BLOCK_DEV_PATH[i]);

#ifdef PC_SIMULATE
		break;
#else
		// device path for SD card is /dev/mmcblkX, same as MMC.
		// check if it is MMC or SD card..
		const char *blkptr = strstr(MTD_BLOCK_DEV_PATH[i], "mmcblk");
		if( blkptr ) {
			int fd;
			char path[64], buf[16];
			memset( path, 0, sizeof(path) );
			sprintf(path, "/sys/block/mmcblk%c/device/type", *(blkptr+strlen("mmcblk")));
			install_info("path(%s)\n", path);
			fd = open(path, O_RDONLY);
			if( fd != -1 ) {
				memset(buf, 0, sizeof(buf));
				read(fd, buf, sizeof(buf) );
				close(fd);
				install_info("path(%s) string(%s)\n", path, buf);
				if( strncmp(buf, "MMC",strlen("MMC"))==0 ) {
					break;
				} else {
					close(dev_fd);
					i++;
				}
			} else {
				install_debug("open path(%s) failed\n", path);
				close(dev_fd);
				i++;
			}
		} else {
			break;
		}
#endif
	} while( i < (int)(sizeof(MTD_BLOCK_DEV_PATH)/sizeof(MTD_BLOCK_DEV_PATH[0]))  );

	if( i < (int)(sizeof(MTD_BLOCK_DEV_PATH)/sizeof(MTD_BLOCK_DEV_PATH[0])) ) {
		if(NULL != ppstr) *ppstr = MTD_BLOCK_DEV_PATH[i];
		pblock_name = MTD_BLOCK_DEV_PATH[i];
		return dev_fd;
	}

	return -1;
}

#define SYS_BLOCK_PATH "/sys/class/block/sata"
#define SATA_PATH "/dev/block/sata"
int get_sata_block_name(char** ppstr)
{
   int len;
   char dev, buff[512]={0};
   char *dev_path;
   struct stat stat_buf;
   
   dev_path = (char*)malloc(32);
   memset(dev_path, 0, 32);
   
   if(pblock_name != NULL)
   {
      printf("[Installer_D]: SATA HDD pblock_name path [%s].\n", pblock_name);
      *ppstr = pblock_name;
      return 0;
   }

   for(dev='a'; dev <= 'z'; dev++){
       snprintf(dev_path, 32, "%s%c", SYS_BLOCK_PATH, dev);
       if ((len = readlink(dev_path, buff, sizeof(buff)-1)) != -1){
           buff[len] = '\0';
           if(strstr(buff, "sata")){
               memset(dev_path, 0, 32);
               snprintf(dev_path, 32, "%s%c", SATA_PATH, dev);
               break;
           }
       }
       else{
          install_log("Open block dev(sata%c) fail\r\n", dev);
          break;
       }
   }
   
   if((dev_path != NULL) && (stat(dev_path, &stat_buf) == 0)){
       printf("[Installer_D]: SATA HDD path [%s].\n", dev_path);
       pblock_name = dev_path;
       *ppstr = dev_path;
       return 0;
   }
   return -1;
}

#else /* else of ifndef __OFFLINE_GENERATE_BIN__ */
int rtk_open_mtd_char(char** ppstr)
{
	pblock_name = MTD_PSEUDO_DEV_PATH;
	return 0;
}

int rtk_open_mtd_block(char** ppstr)
{
    pblock_name = MTD_PSEUDO_DEV_PATH;
    return 0;
}

int get_sata_block_name(char** ppstr)
{
    //MP tool is not support SATA case.
    //so default return -1.
    return -1;
}
#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */

#ifdef EMMC_SUPPORT
int rtk_mmc_write(unsigned int offset, char *data, int size)
{
	int dev_fd;

	if (pblock_name == NULL)
        return -1;

	dev_fd = open(pblock_name, O_RDWR|O_SYNC);
    if(dev_fd < 0)
    {
      install_debug("open mmc block fail\r\n");
      return -1;
    }

	lseek(dev_fd, offset*512, SEEK_SET);
	write(dev_fd, data, size*512);
    close(dev_fd);

	return 0;
}

int rtk_get_mbr(unsigned int offset)
{
#if (1)	//Chuck TODO
	return 0;
#else
   	char command[128];
	int ret=0;

	if (pblock_name == NULL)
		return -1;

   	sprintf(command, "dd if=%s of=%s skip=%d bs=512 count=1", pblock_name, MBR_FILE_TEMP, offset);
   	ret=rtk_command(command, __LINE__, __FILE__);

	return ret;
#endif
}

int rtk_set_mbr(unsigned int blkoffset, char *mbr)
{
    int ret=0, dev_fd;
	off64_t off = (off64_t)blkoffset*512;;

    if (pblock_name == NULL)
        return -1;

#ifdef PC_SIMULATE
	static int idx=0;
	char path[32];

	if( idx==0 ) {
		rtk_command("rm -f tmp/mbr*", __LINE__, __FILE__);
	}

	// write mbr
	sprintf(path, "tmp/mbr_%02d.bin", idx);
	if( (dev_fd = open(path, O_RDWR|O_SYNC|O_CREAT, 0644)) < 0 ) {
		install_fail("open %s failed\n", path);
		return -1;
	}
	idx++;
#else
	// re-read the partition table, and trigger hotplug to cread related device nodes
	dev_fd = open(pblock_name, O_RDWR);
	if (dev_fd < 0)
    {
    	install_debug("open %s block fail\r\n", pblock_name);
      	return -1;
   	}

	// lseek() is not enough, and we need lseek64()
	ret = lseek64(dev_fd, off, SEEK_SET);
	if (ret == (off64_t) -1) {
		install_fail("lseek %llx failed\r\n",off);
		return ret;
	}
#endif

	write(dev_fd, mbr, 512);

#ifndef __OFFLINE_GENERATE_BIN__
	if( (ret=ioctl(dev_fd, BLKRRPART)) < 0 ) {
		printf("ioctl BLKRRPART error, errno(%d)[%s]\n", errno, strerror(errno));
	}
#endif

	close(dev_fd);

    return ret;
}

#ifndef __OFFLINE_GENERATE_BIN__
unsigned long rtk_get_size_emmc(void)
{
	int fd;
	unsigned long long v64;
    unsigned long longsectors;

	if (pblock_name == NULL)
        return -1;

	fd = open(pblock_name, O_RDWR|O_SYNC);
    if(fd < 0)
    {
      install_fail("open mmc block %s fail\r\n", pblock_name);
      return -1;
    }

    if (ioctl(fd, BLKGETSIZE64, &v64) == 0) {
        /* Got bytes, convert to 512 byte sectors */
        v64 >>= 9;
        if (v64 != (unsigned long)v64) {
 ret_trunc:
            /* Not only DOS, but all other partition tables
             * we support can't record more than 32 bit
             * sector counts or offsets
             */
            install_fail("device has more than 2^32 sectors, can't use all of them");
            v64 = (unsigned long)-1L;
        }
		close(fd);
        return v64;
    }
    /* Needs temp of type long */
    if (ioctl(fd, BLKGETSIZE, &longsectors)) {
        /* Perhaps this is a disk image */
        unsigned long sz = lseek(fd, 0, SEEK_END);
        longsectors = 0;
        if (sz > 0)
            longsectors = (unsigned long)sz / 512;
        lseek(fd, 0, SEEK_SET);

    }
    if (sizeof(long) > sizeof(unsigned long)
     && longsectors != (unsigned long)longsectors
    ) {
        goto ret_trunc;
    }
	close(fd);
    return longsectors;
}

int rtk_mmcboot_process(void)
{
	char bootdev0[50], bootdev1[50];
	int fd, ret = 0;
	unsigned int offset = 0x20800, magic = 0xffffffff, data=0;

   if (pblock_name == NULL)
   	return -1;

	sprintf(bootdev0, "%sboot0", pblock_name);
	sprintf(bootdev1, "%sboot1", pblock_name);

	// So far, we don't put bootcode in boot part of emmc
	// fill 0xFFFF_FFFF into bootpart0 to allow romcode to skip parsing
   fd = open(bootdev0, O_RDWR|O_SYNC);
   if(fd < 0) {
   	install_fail("open %s fail\r\n", bootdev0);
      return -1;
   }

	ret = lseek(fd, offset, SEEK_SET);
   if(ret < 0)
   {
      install_fail("lseek %s fail\r\n", bootdev0);
		close(fd);
      return -1;
   }

	ret = write(fd, &magic, sizeof(magic));
   if(ret != 4)
   {
      install_fail("write %s fail\r\n", bootdev0);
      close(fd);
      return -1;
   }

	// read it back to verify
   ret = lseek(fd, offset, SEEK_SET);
   if(ret < 0)
   {
      install_fail("lseek %s fail\r\n", bootdev0);
      close(fd);
      return -1;
   }

	ret = read(fd, &data, sizeof(data));
	if ((ret != 4) || (data != magic)) {
	   install_fail("verify %s fail, we need %x but %x actually\r\n", bootdev0, data, magic);
      close(fd);
      return -1;

	}
	close(fd);

	data = 0;
	// fill 0xFFFF_FFFF into bootpart1 to allow romcode to skip parsing
	fd = open(bootdev1, O_RDWR|O_SYNC);
   if(fd < 0) {
      install_fail("open %s fail\r\n", bootdev1);
      return -1;
   }

   ret = lseek(fd, offset, SEEK_SET);
   if(ret < 0)
   {
      install_fail("lseek %s fail\r\n", bootdev1);
      close(fd);
      return -1;
   }

   ret = write(fd, &magic, sizeof(magic));
   if(ret != 4)
   {
      install_fail("write %s fail\r\n", bootdev1);
      close(fd);
      return -1;
   }

	// read it back to verify
   ret = lseek(fd, offset, SEEK_SET);
   if(ret < 0)
   {
      install_fail("lseek %s fail\r\n", bootdev1);
      close(fd);
      return -1;
   }

   ret = read(fd, &data, sizeof(data));
   if ((ret != 4) || (data != magic)) {
      install_fail("verify %s fail, we need %x but %x actually\r\n", bootdev1, data, magic);
      close(fd);
      return -1;

   }

   	close(fd);
	return 0;
}
#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */
#endif //EMMC_SUPPORT


int get_mtd_block_name(char** ppstr)
{
   int ret;
   char* p;
   if(pblock_name != NULL)
   {
      *ppstr = pblock_name;
      return 0;
   }
   ret = rtk_open_mtd_block(&p);
   if(ret < 0)
   {
      return -1;
   }
   close(ret);
   *ppstr = p;
   return 0;
}

char* get_mtd_block_name_str(void)
{
    int ret;
    char* p;
    ret = rtk_open_mtd_block(&p);
    if(ret < 0)
    {
        return NULL;
    }
    close(ret);
    return p;
}

int rtk_open_mtd_block_with_offset(unsigned int offset)
{
   int i;
   int dev_fd;
   int ret;
#if defined(NAS_ENABLE) && !defined(PC_SIMULATE)
   char *mtd_name;
   dev_fd = rtk_open_last_mtd("/dev/mtdblock", &mtd_name);
   if(dev_fd >= 0){
       pblock_name = mtd_name;
       install_info("[NAS]Open MTD_BLOCK %s with %u(0x%08X) offset\r\n",
                    mtd_name, offset, offset);
       ret = lseek(dev_fd, offset, SEEK_SET);
       if(ret < 0)
       {
          install_debug("[NAS]lseek mtd block fail\r\n");
          return -1;
       }

       return dev_fd;
   }
#endif

   for(i=0;i<(int)(sizeof(MTD_BLOCK_DEV_PATH)/sizeof(MTD_BLOCK_DEV_PATH[0]));i++)
   {
      dev_fd = open(MTD_BLOCK_DEV_PATH[i], O_RDWR|O_SYNC);
      if(dev_fd != -1) break;
   }
   if(dev_fd < 0)
   {
      install_debug("open mtd block fail\r\n");
      return -1;
   }
   pblock_name = MTD_BLOCK_DEV_PATH[i];
   install_info("Open MTD_BLOCK %s with %u(0x%08X) offset\r\n", MTD_BLOCK_DEV_PATH[i], offset, offset);
   ret = lseek(dev_fd, offset, SEEK_SET);
   if(ret < 0)
   {
      install_debug("lseek mtd block fail\r\n");
      return -1;
   }

   return dev_fd;
}

int rtk_get_meminfo(struct mtd_info_user* meminfo)
{
   int dev_fd;
   dev_fd = rtk_open_mtd_char();
   if(-1 == dev_fd)
      return -1;
   /* Get MTD device capability structure */
   if (ioctl(dev_fd, MEMGETINFO, meminfo))
   {
      close(dev_fd);
      return -1;
   }

   close(dev_fd);
   return 0;
}

#ifndef __OFFLINE_GENERATE_BIN__
uint32_t rtk_get_erasesize(void)
{
   int ret;
   struct mtd_info_user meminfo;

   if((ret = rtk_get_meminfo(&meminfo)) < 0)
   {
      return 0;
   }
   else
      return meminfo.erasesize;
}
#else /* else of ifndef __OFFLINE_GENERATE_BIN__ */
uint32_t rtk_get_erasesize(void)
{
#if 1
   install_test("do not call rtk_get_erasesize()\r\n");
   return 0x1000;
#else
   return 0x1000;
#endif
}
#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */

int modify_addr_signature(unsigned int startAddress, unsigned int reserved_boot_size)
{
   int dev_fd;
   int ret;
   dev_fd = rtk_open_mtd_block_with_offset(reserved_boot_size);
   if(dev_fd < 0)
   {
      install_debug("rtk_open_mtd_block_with_offset() fail\r\n");
      return -1;
   }
   ret = write(dev_fd, "IMG_", 4);
   if(ret != 4)
   {
      install_debug("write signature fail\r\n");
      close(dev_fd);
      return -1;
   }
   ret = write(dev_fd, (const void *) &startAddress, 4);
   if(ret != 4)
   {
      install_debug("modify signature's num fail\r\n");
      close(dev_fd);
      return -1;
   }
   close(dev_fd);
   return 0;
}

int modify_signature(unsigned int reserved_boot_size, unsigned int flash_type)
{
   char* block_path;
   int ret;
   char command[128];
   ret = get_mtd_block_name(&block_path);
   if(ret < 0)
   {
      install_debug("get_mtd_block_name fail\r\n");
      return -1;
   }
   if(flash_type == MTD_NANDFLASH || flash_type == MTD_EMMC)
   {
      // modify the signature (first 8 bytes of boot table)
      sprintf(command, "echo -n RESCUE__ | dd of=%s bs=1 seek=%u", block_path, reserved_boot_size);
      ret = rtk_command(command, __LINE__, __FILE__);
   }
   else if(flash_type == MTD_NORFLASH || flash_type == MTD_DATAFLASH)
   {
      // modify the signature (first 8 bytes of boot table)
#ifdef CONFIG_BOOT_FROM_SPI
      sprintf(command, "echo -n RESCUE__ | dd of=%s bs=1 seek=%u", block_path, reserved_boot_size);
#else
      sprintf(command, "echo -n RESCUE__ | dd of=%s bs=1 seek=0", block_path);
#endif
      ret = rtk_command(command, __LINE__, __FILE__);
   }
   else
   {
      install_debug("Unknown MTD TYPE\r\n");
      return -1;
   }
   return 0;
}
