#ifndef RTK_DEF
#define RTK_DEF

// 1) if the flash size returned by ioctl(MEMGETINFO) is the actual NAND flash
//    size, we've to reserve 5% for BB remapping.
// 2) if the returned size already exclude the BB remapping size, we don't need
//    to reserve it.
// define RESERVED_AREA if it is case 1) above.
//#define RESERVED_AREA

// 1) if system has /sys/realtek_boards/system_parameters, we can read it to get
//    some system parameters( factorSize, bootcodeSize, etc..)
// 2) otherwise, system parameters are determined by nand blockSize/pageSize
// define USING_SYSTEM_FILE if it is case 1)
//#define USING_SYSTEM_FILE

#ifndef __OFFLINE_GENERATE_BIN__
/**/
#ifndef PC_SIMULATE
#define PKG_TEMP                                  "/tmp"
#else
#define PKG_TEMP                                  "tmp"
#endif
#if 0
#define JFFS2_BIN                 "/usr/sbin/mkfs.jffs2"
#define YAFFS2_BIN                "/usr/sbin/mkyaffs2image"
#define FLASHERASE_BIN            "/usr/sbin/flash_erase"
#define NANDWRITE_BIN             "/usr/sbin/nandwrite"
#else
#define JFFS2_BIN                                 "/tmp/mkfs.jffs2"
#define YAFFS2_BIN                                "/tmp/mkyaffs2image"
#define FLASHERASE_BIN                            "/tmp/flash_erase"
#define NANDWRITE_BIN                             "/tmp/nandwrite"
#define UBIFORMAT_BIN                             "/tmp/ubiformat"
#define MKE2FS_BIN                             	  "/tmp/mke2fs"
#define RESIZE2FS_BIN                             "/tmp/resize2fs"

#define ALSADAEMON_BIN                            "/tmp/alsadaemon"
#define FWDESCTABLE                               "fw_tbl.bin"
#define GOLD_FWDESCTABLE                          "gold_fw_tbl.bin"
#define RESCUEFWDESCTABLE                         "resc_fw_tbl.bin"

#define KEY_RSA_BIN                               "/tmp/Krsa.bin"
#define KEY_KSEED_BIN                             "/tmp/Kseed.bin"
#define KEY_K_BIN                                 "/tmp/Key.bin"
#define KEY_K1_BIN                                "/tmp/Key1.bin"
#define KEY_K2_BIN                                "/tmp/Key2.bin"
#define KEY_K3_BIN                                "/tmp/Key3.bin"

#define TEE_UTILITY                               "/tmp/teeUtility.tar"
#endif

#define EXTRACTED_FILE_TEMP                       PKG_TEMP"/_temp_extract_"
//#define BOOT_CODE_DIR                             "/tmp"


#define BIN                                       "/bin"

#define RTK_NAND_HWSETTING_START_BLOCK           6

#else /* else of ifndef __OFFLINE_GENERATE_BIN__ */
/**/
#define DEFAULT_OUTPUT_DIR                        "output"
#define DEFAULT_OUTPUT                            DEFAULT_OUTPUT_DIR"/output.bin"
#define DEFAULT_TEMP_OUTPUT                       DEFAULT_OUTPUT_DIR"/temp_output"
#define DEFAULT_ENDIAN_SWAP_TEMP                  DEFAULT_OUTPUT_DIR"/_temp_swap"
#ifdef EMMC_SUPPORT
	#define DEFAULT_BP1_OUTPUT								  DEFAULT_OUTPUT_DIR"/output.bp1.bin"
	#define DEFAULT_BP2_OUTPUT								  DEFAULT_OUTPUT_DIR"/output.bp2.bin"
	#define DEFAULT_EXT4_OUTPUT							  DEFAULT_OUTPUT_DIR"/ext4.bin"
	#define DEFAULT_FW_OUTPUT							  	  DEFAULT_OUTPUT_DIR"/fw.bin"
#endif

#define PKG_TEMP                                  "tmp"
#define JFFS2_BIN                                 "./mkfs.jffs2"
#define YAFFS2_BIN                                "./mkyaffs2image"
#define FLASHERASE_BIN                            "./flash_erase"
#define NANDWRITE_BIN                             "./nandwrite"
#define UBIFORMAT_BIN                             "./ubiformat"
#define MKE2FS_BIN                                "./mke2fs"

#define ALSADAEMON_BIN                            "alsadaemon"
#define FWDESCTABLE                               "fw_tbl.bin"
#define GOLD_FWDESCTABLE                          "gold_fw_tbl.bin"
#define FACTORYBLOCK                              "factory.bin"
#define BIN                                       "bin"

//setting file
#define DEFAULT_SETTING_FILE                      "setting.txt"
#define SETTING_STRING_IMAGE_OUTPUT               "CONFIG_IMAGE_OUTPUT"
#define SETTING_STRING_FLASH_PARTNAME             "CONFIG_FLASH_PARTNAME"
#define SETTING_STRING_FLASH_TYPE                 "CONFIG_FLASH_TYPE"
#define SETTING_STRING_FLASH_SIZE_KB              "CONFIG_FLASH_SIZE_KB"
#define SETTING_STRING_FLASH_ERASE_SIZE_KB        "CONFIG_FLASH_ERASE_SIZE_KB"
#define SETTING_STRING_FLASH_PAGE_SIZE            "CONFIG_FLASH_PAGE_SIZE"
#define SETTING_STRING_FLASH_OOB_SIZE             "CONFIG_FLASH_OOB_SIZE"

#define SETTING_STRING_FACTORY_SIZE_AUTO          "CONFIG_FACTORY_SIZE_AUTO"
#define SETTING_STRING_FACTORY_SIZE_MANUAL        "CONFIG_FACTORY_SIZE_MANUAL"
#define SETTING_STRING_FACTORY_SIZE               "CONFIG_FACTORY_SIZE"

#define SETTING_STRING_FLASH_PROGRAMMER_MODEL     "CONFIG_FLASH_PROGRAMMER_MODEL"
#define SETTING_STRING_FLASH_PROGRAMMER_NAME      "CONFIG_FLASH_PROGRAMMER_NAME"

#ifdef EMMC_SUPPORT
	#define SETTING_STRING_FLASH_BP1_SIZE_KB		  "CONFIG_FLASH_BP1_SIZE_KB"
	#define SETTING_STRING_FLASH_BP2_SIZE_KB		  "CONFIG_FLASH_BP2_SIZE_KB"
	#define SETTING_STRING_FLASH_ERASED_CONTENT	  "CONFIG_FLASH_ERASED_CONTENT"
#endif



#define RTK_NAND_HWSETTING_START_BLOCK           6

//nand block state
#define RTK_NAND_BLOCK_STATE_GOOD_BLOCK          0xff
#define RTK_NAND_BLOCK_STATE_BAD_BLOCK           0x00
#define RTK_NAND_BLOCK_STATE_BAD_BLOCK_TABLE     0xbb
#define RTK_NAND_BLOCK_STATE_HW_SETTING          0x23
#define RTK_NAND_BLOCK_STATE_BOOTCODE            0x79
#define RTK_NAND_BLOCK_STATE_RESCUE_N_LOGO       0x80
#define RTK_NAND_BLOCK_STATE_EVN_VAR             0x81
#define RTK_NAND_BLOCK_STATE_FACTORY             0x82

#define SECURE_FSBL_MAGIC_NUM   0x89        /* Identify secure boot loader in NAND flash spare area*/
#define SECURE_OS_MAGIC_NUM     0x90        /* Identify secure os loader in NAND flash spare area*/
#define BL31_MAGIC_NUM          0x91        /* Identify bl31 in NAND flash spare area*/
#define RSA_KEY_FW_MAGIC_NUM    0x92        /* Identify rsa key fw in NAND flash spare area*/
#define RSA_KEY_TEE_MAGIC_NUM   0x93        /* Identify rsa key tee in NAND flash spare area*/

//nand rtk
#define RTK_UNIT_PAGE_SIZE                        512
#define RTK_UNIT_OOB_SIZE                         16
#define RTK_NAND_ECC_SIZE_PER_UNIT_PAGE           10

//programmer
#define DEFAULT_PROGRAMMER_DEF_FILE               DEFAULT_OUTPUT_DIR"/temp_def"

#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */

#define NAND_BOOT_BACKUP_COUNT                    4 // number of backup bootcode in NAND flash
#define FLASH_MAGICNO_NAND                        0xce
#define BOOTCODE_DDR_BASE                         0xa0020004	// bootcode address on DDR

#define NAND_DEFAULT_FACTORY_START                0xc00000
#define NAND_DEFAULT_RBA_PERCENTAGE               5
#define NAND_DEFAULT_CONFIG_FACTORY_SIZE          0x400000


#define NORBOOTCODE_TEMP                          PKG_TEMP"/bootloader_nor"
#define NORBOOTCODE2_TEMP                         PKG_TEMP"/bootloader2_nor"
#define NOR_NORMAL_DTB_TEMP                       NORBOOTCODE2_TEMP"/android.spi.dtb"
#define NOR_RESCUE_DTB_TEMP                       NORBOOTCODE2_TEMP"/rescue.spi.dtb"
#define BOOTCODE_TEMP                             PKG_TEMP"/bootloader"
#define FACTORY_TEMP                              PKG_TEMP"/factory"
#define FACTORY_INSTALL_TEMP                      PKG_TEMP"/install_factory"
#define EXTRACTED_FILE_TEMP                       PKG_TEMP"/_temp_extract_"
#define POSTPROCESS_TEMP                       	  PKG_TEMP"/postprocess.sh"
#define OTP_KEY_VERIFY_TEMP						  PKG_TEMP"/otp_key_verify.aes"

//project_config.h
#ifdef EMMC_SUPPORT
	#define PROJECT_CONFIG_FILE                         "sys_param.h"
	#define PROJECT_CONFIG_STRING_CONFIG_FACTORY_SIZE   "SYS_PARAM_FACTORY_SIZE"
	#define PROJECT_CONFIG_STRING_CONFIG_FACTORY_START  "SYS_PARAM_FACTORY_START"
	#define PROJECT_CONFIG_STRING_CONFIG_BOOTCODE_START "SYS_PARAM_BOOTCODE_START"
	#define MBR_RESERVE_SIZE									 8192
#else	// for NAND
	#define PROJECT_CONFIG_FILE                       "project_config.h"
	#define PROJECT_CONFIG_STRING_CONFIG_FACTORY_SIZE "CONFIG_FACTORY_SIZE"
	#define PROJECT_CONFIG_STRING_RTK_RBA_PERCENTAGE  "RTK_RBA_PERCENTAGE"
#endif

#ifdef PC_SIMULATE
#define SYSTEM_PARAMETERS                         "system_parameters"
#define ioctl ioctl_pc
extern int ioctl_pc(int d, int request, ...);

#define DEFAULT_SETTING_FILE                      "setting.txt"
#define SETTING_STRING_IMAGE_OUTPUT               "CONFIG_IMAGE_OUTPUT"
#define SETTING_STRING_FLASH_PARTNAME             "CONFIG_FLASH_PARTNAME"
#define SETTING_STRING_FLASH_TYPE                 "CONFIG_FLASH_TYPE"
#define SETTING_STRING_FLASH_SIZE_KB              "CONFIG_FLASH_SIZE_KB"
#define SETTING_STRING_FLASH_ERASE_SIZE_KB        "CONFIG_FLASH_ERASE_SIZE_KB"
#define SETTING_STRING_FLASH_PAGE_SIZE            "CONFIG_FLASH_PAGE_SIZE"
#define SETTING_STRING_FLASH_OOB_SIZE             "CONFIG_FLASH_OOB_SIZE"

#define SETTING_STRING_FACTORY_SIZE_AUTO          "CONFIG_FACTORY_SIZE_AUTO"
#define SETTING_STRING_FACTORY_SIZE_MANUAL        "CONFIG_FACTORY_SIZE_MANUAL"
#define SETTING_STRING_FACTORY_SIZE               "CONFIG_FACTORY_SIZE"

#define SETTING_STRING_FLASH_PROGRAMMER_MODEL     "CONFIG_FLASH_PROGRAMMER_MODEL"
#define SETTING_STRING_FLASH_PROGRAMMER_NAME      "CONFIG_FLASH_PROGRAMMER_NAME"

#ifdef EMMC_SUPPORT
	#define SETTING_STRING_FLASH_BP1_SIZE_KB		  "CONFIG_FLASH_BP1_SIZE_KB"
	#define SETTING_STRING_FLASH_BP2_SIZE_KB		  "CONFIG_FLASH_BP2_SIZE_KB"
	#define SETTING_STRING_FLASH_ERASED_CONTENT	  "CONFIG_FLASH_ERASED_CONTENT"
#endif
#else
#define SYSTEM_PARAMETERS                         "/sys/realtek_boards/system_parameters"
#endif
#define RTKIMGDESCTXT_PATH                        "/tmp/rtkimgdesc.txt"

//extern parameter
#define DEFAULT_MAC_ADDRESS_HI                    0x33221100
#define DEFAULT_MAC_ADDRESS_LO                    0x55440000

//file in bootcode.tar
#define NOR_HWSETTING_START_ADDR                  0x3000
#define RESETROM_FILENAME                         "resetrom-new.bin"
#define HWSETTING_FILENAME                        "hw_setting.bin"
#define HASHTARGET_FILENAME                       "hash_target.bin"
#define NAND_RESCUE_FILENAME                      "rescue.bin"
#define RESCUE_FILENAME                           "vmlinux.rescue.macarthur.nonet.bin.lzma"
#define FIND_RESCUE_FILENAME                      "vmlinux.rescue"
#define UBOOT_FILENAME                            "uboot.bin"
#define UBOOT2_HDR                                "bootcode2.hdr"
#define UBOOT2_FILENAME                           "uboot2.bin"
#define FULL_FILENAME                             "full.bin"
#define NORBOOTCODE_FULL_TMP                      NORBOOTCODE_TEMP"/"FULL_FILENAME
#define FULL_UBOOT2                               "uboot2_full.bin"
#define NORBOOTCODE2_BIN_TMP                      NORBOOTCODE2_TEMP"/"FULL_UBOOT2

//Used for customer or UI
#define CUSTOMER_BINARY                     "customer"
#define CUSTOMER_PATH                           PKG_TEMP"/"CUSTOMER_BINARY
#define CUSTOMER_BIN                              BIN"/"CUSTOMER_BINARY
#define VIDEO_FW_FILENAME                         "video_firmware.install.bin"
#define FONT_FILENAME                             "font.ttf"
#define VIDEO_FW_FILENAME_PATH                    PKG_TEMP"/"VIDEO_FW_FILENAME
#define FONT_FILENAME_PATH                        PKG_TEMP"/"FONT_FILENAME

#define WAIT_CUSTOMER_LOCKFILE                    PKG_TEMP"/customer.lock"

//LOGO
#define CUSTOMER_AUDIO_LOGO_DEFAULT_SIZE          0x180000
#define CUSTOMER_VIDEO_LOGO_DEFAULT_SIZE          0x280000
#define CUSTOMER_AUDIO_LOGO_DEFAULT_SIZE_KB       1536
#define CUSTOMER_VIDEO_LOGO_DEFAULT_SIZE_KB       2560

#define CUSTOMER_AUDIO_LOGO_DEFAULT_TARGET_ADDRESS 0x83000000
#define CUSTOMER_VIDEO_LOGO_DEFAULT_TARGET_ADDRESS 0x83100000

#define CUSTOMR_AUDIO_LOGO1_BACKUP_FILENAME       "calogo1.bak"
#define CUSTOMR_VIDEO_LOGO1_BACKUP_FILENAME       "cvlogo1.bak"
#define CUSTOMR_AUDIO_LOGO2_BACKUP_FILENAME       "calogo2.bak"
#define CUSTOMR_VIDEO_LOGO2_BACKUP_FILENAME       "cvlogo2.bak"


// Factory
#define BOOTPARAM_FILENAME                        "000BootParam.h"
#define LAYOUT_FILENAME                           "layout.txt"
#define UPGRADE_LOCK_FILENAME                     "upgrade.lock"
#define UPDATE_BOOTCODE_FILENAME                  "update_bootcode"
#define ENVSETTINGS_FILENAME					  "env.txt"
#define USB_UPDATE_FLAG_FILENAME                  "updateflags.dat"

#define CREATE_FILE_FACTORY_MODE                  1
#define REMOVE_FILE_FACTORY_MODE                  2

// Files could not be removed when install factory
const char essential_files_in_factory[][20] = {
   LAYOUT_FILENAME,
   LAYOUT_FILENAME"_md5"
};

#define CP15DMB   asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))

#endif /* End of RTK_DEF */
