#ifndef RTK_MTD_H
#define RTK_MTD_H
// flash meminfo api
// flash open api
// command api
#include <sys/types.h>
#include <stdlib.h>
#include <rtk_common.h>
#define MEMGETINFO       _IOR('M', 1, struct mtd_info_user)
#define MEMUNLOCK	 _IOW('M', 6, struct erase_info_user)

#define GETRBAPERCENTAGE  _IOR('M', 34, unsigned int)

// for install speed up
#define MEMERASE         _IOW('M', 2, struct erase_info_user)
#define RTKMEMERASE      _IO('M', 20)
#define RTKINSTALLREAD   _IO('M', 31)
#define RTKINSTALLWRITE  _IO('M', 32)
#define RTKINSTALLERASE  _IO('M', 33)

#ifdef EMMC_SUPPORT
#define BLKRRPART  _IO(0x12,95) /* re-read partition table */
#define BLKGETSIZE _IO(0x12,96) /* return device size in bytes*/
#define BLKGETSIZE64 _IOR(0x12,114,size_t) /* return device size in bytes (u64)*/
#endif

#define MTD_ABSENT              0
#define MTD_RAM                 1
#define MTD_ROM                 2
#define MTD_NORFLASH            3
#define MTD_NANDFLASH           4
#define MTD_PEROM               5
#define MTD_DATAFLASH           6
#define MTD_EMMC 	            7
#define MTD_SATA                8
#define MTD_OTHER               14
#define MTD_UNKNOWN             15

#ifdef EMMC_SUPPORT
int rtk_mmc_write(unsigned int offset, char *data, int size);
int rtk_get_mbr(unsigned int offset);
int rtk_set_mbr(unsigned int blkoffset, char *mbr);
unsigned long rtk_get_size_emmc(void);
int rtk_mmcboot_process(void);
#endif
int rtk_open_mtd_char(char** p=NULL);
int rtk_open_mtd_block(char** p=NULL);
int get_mtd_block_name(char** ppstr);
int rtk_get_meminfo(struct mtd_info_user* meminfo);
int rtk_get_meminfo_1(struct mtd_info_user_1* meminfo_1);
uint32_t rtk_get_erasesize(void);
char* get_mtd_block_name_str(void);

int rtk_open_mtd_block_with_offset(unsigned int offset);

int modify_addr_signature(unsigned int startAddress, unsigned int reserved_boot_size);
int modify_signature(unsigned int reserved_boot_size, unsigned int flash_type);
int get_sata_block_name(char** ppstr);
#endif

