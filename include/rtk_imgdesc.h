#ifndef RTK_IMGDESC_H
#define RTK_IMGDESC_H
#include <rtk_urltar.h>
#include <rtk_tar.h>
#include <stdio.h>

#define NUM_RTKPART (FW_USR_LOCAL_ETC-FW_ROOTFS+1)
#define RTKIMAGE_DESC_MAGICNUM  0xF1F2F3F4
#define CONFIGURATION_MAX_SIZE 10*1024
#define NUM_SUPPORT_DYNAMIC_PART_TBL 4
//#define ENABLE_ERASE_CHECK
#define DEBUG_FUNC
#define DEV_PATH_NAND_MTD_SP    "/dev/mtd/mtd1"
#define DEV_PATH_NAND_BLOCK_SP  "/dev/block/mtdblock1"

enum FWTYPE
{
   FW_KERNEL=0,
   FW_RESCUE_DT,
   FW_KERNEL_DT,
   FW_RESCUE_ROOTFS,
   FW_KERNEL_ROOTFS,
   FW_NAS_KERNEL,
   FW_NAS_RESCUE_DT,
   FW_NAS_RESCUE_ROOTFS,
   FW_AUDIO,
   FW_TEE,
   FW_VIDEO, //10
   FW_VIDEO2,
   FW_ECPU,
   FW_AUDIO_BOOTFILE,
   FW_IMAGE_BOOTFILE,
   FW_VIDEO_BOOTFILE,
   FW_AUDIO_CLOGO1,
   FW_IMAGE_CLOGO1,
   FW_VIDEO_CLOGO1,
   FW_AUDIO_CLOGO2,
   FW_IMAGE_CLOGO2, //20
   FW_VIDEO_CLOGO2,
   FW_ROOTFS,
   FW_RES,
   FW_CACHE,
   FW_DATA,
   FW_SYSTEM,
   P_PARTITION1, //27
   P_PARTITION2,
   P_PARTITION3,
   P_PARTITION4,
   P_PARTITION5,
   P_PARTITION6,
   P_PARTITION7,
   P_PARTITION8,
   FW_USR_LOCAL_ETC,
   FW_BOOTCODE,
   FW_FACTORY,
   FW_FW_TBL,
   P_SROOTFS,
   FW_RESCUE,  //40
   FW_P_SSU_WORKING,
   FW_P_FREE_SPACE,
   FW_NORBOOTCODE,
   FW_GOLD_RESCUE_DT, //44
   FW_GOLD_AUDIO,
   FW_GOLD_RESCUE_ROOTFS,
   FW_GOLD_KERNEL,
   FW_GOLD_FW_TBL,
   FW_UBOOT, //49
   FW_SE_STORAGE, //50
   FW_BL31,
   FW_XEN,
   FW_UNKNOWN
};

enum UTILTYPE
{
   UTIL_FLASHERASE,
   UTIL_NANDWRITE,
   UTIL_UBIFORMAT,
   UTIL_MKE2FS,
   UTIL_RESIZE2FS,
   UTIL_MKYAFFS2,
   UTIL_MKJFFS2,
   UTIL_MAX
};

enum KEYTYPE
{
   KEY_RSA_PUBLIC,
   KEY_AES_KEY_SEED,
   KEY_AES_KEY,
   KEY_AES_KEY1,
   KEY_AES_KEY2,
   KEY_AES_KEY3,
   KEY_MAX
};

enum eRTK_IMAGE_STATUS
{
   rUNINSTALL,
   rUPGRADING,
   rCOMPLETED
};

enum eIMAGE_STATUS
{
   iUNINSTALL,
   iINSTALLING,
   iCOMPLETED
};

enum eMode
{
	_INSTALL,
	_UPGRAD,
};

enum eFS
{
	FS_TYPE_JFFS2,
	FS_TYPE_YAFFS2,
	FS_TYPE_SQUASH,
	FS_TYPE_RAWFILE,
	FS_TYPE_EXT4,
	FS_TYPE_UBIFS,
	FS_TYPE_NONE,
	FS_TYPE_UNKOWN
};

enum eBURN
{
	BURN_BY_NANDWRITE,
	BURN_BY_MTDBLOCK,
	BURN_BY_MMCBLOCK,
};


struct t_PARTDESC
{
	FWTYPE efwtype;
	char partition_name[32];
	char mount_point[32];
	eFS efs;
	char filename[32];
	unsigned long long min_size;
#ifdef ENABLE_ERASE_CHECK
	unsigned int bErase;   // 1: erase before updated; 0: not erase
#endif
	char mount_dev[32];
};

struct t_DYNAMIC_PARTDESC
{
	unsigned int flash_sizeM;
	struct t_PARTDESC partDescTbl[NUM_RTKPART];
};

struct t_FWDESC
{
	FWTYPE efwtype;
	char firmware_name[32];
	char filename[64];
	unsigned int target;
};

struct t_UTILDESC
{
	UTILTYPE type;
	const char *pattern;
	const char *bin_path;
};

struct t_KEYDESC
{
	KEYTYPE type;
	const char *pattern;
	const char *bin_path;
};

enum ePARTIAL_INSTALL
{
   PARTIAL_NOT_PASS=0,
   PARTIAL_PASS
};

enum eGUI_CONFIG
{
   GUI_ENABLE=0,
   GUI_DISABLE
};

enum eINSTALL_MODE
{
   eUPDATE = 0,
   eBACKUP,
   eSKIP,
   eFIRSTTIME,		// there was no this image before, and we need to something at the first time.
};

struct t_imgdesc
{
   enum FWTYPE efwtype;
   /* install status */
   enum ePARTIAL_INSTALL pass[2];
   /* current install offset */
   unsigned int install_offset;
   /* file posotion*/
   unsigned int tarfile_offset;
   unsigned int img_size;
   unsigned long long flash_allo_size;
   /* redundant info*/
   unsigned long long sector;
   /* flash position*/
   unsigned long long flash_offset;
   /* memory target position*/
   unsigned int mem_offset;
   /* checksum */
   unsigned int checksum;
   unsigned char sha_hash[32];
   unsigned int extracted;
#ifdef ENABLE_ERASE_CHECK
   unsigned int erased;
#endif
   //fw compress type
   char compress_type[16];

   //partition info
   char part_name[32];
   char fs_name[16];

   //mount point
   char mount_point[64];
   char dev_path[64];
   char filename[100];
#ifdef EMMC_SUPPORT
    char emmc_partIdx;
#endif
   //mode
   enum eINSTALL_MODE install_mode;
   char backup_filename[100];

};
//__attribute__((packed));

struct t_rtkimgdesc
{
	enum eRTK_IMAGE_STATUS status;
	unsigned int magic;
	unsigned int ver;
	enum eMode mode;
	enum eGUI_CONFIG egui;
	const char* mtdblock_path;
	const char* mtd_path;
	int next_ssu_work_part;
#ifdef __OFFLINE_GENERATE_BIN__
	char flash_partname[64];
	char flash_programmer_name[128];
	char flash_programmer_model[128];
	int epmtype;
	unsigned int flash_reserved_area_block_count;
	unsigned int flash_block_count;

	char* setting_path;
	char* output_path;
	unsigned int byte_swap_off;
	unsigned int remove_temp_file_off;
	unsigned int whole_image_on;
	unsigned int all_in_one;	// merge all partition into one big image or into a separated file
	unsigned int bAndroid_path;   // This is used for identify which block path should be written into layout.txt while writebootable in off-line generator
	unsigned int target_flashM;
#endif
	unsigned int norflash_size_32MB;

	// config info
	// yes: 1    no: 0
	unsigned int bootcode;
	unsigned int install_factory;
	unsigned int install_dtb;
	unsigned int verify;
	unsigned int ignore_native_rescue;
	unsigned int stop_reboot;
	unsigned int update_etc;
	unsigned int start_customer;
	unsigned int ap_customer;
	unsigned int kill_000;
	unsigned int partition_inverse;
	unsigned int only_factory;
	unsigned int only_bootcode;
	unsigned int safe_upgrade;
	unsigned int erase_free_space;
	unsigned int jffs2_nocleanmarker;
	unsigned int dump_flash;
	unsigned int update_cavfile;
	unsigned int backup;
#ifdef NAS_ENABLE
	unsigned int nas_rescue;
#endif

	// var
	int install_avfile_count;
	int install_avfile_audio_size; //size in KB
	int install_avfile_video_size; //size in KB
	int reboot_delay;
	int customer_delay;
	int rba_percentage;
	int logger_level;
	int secure_boot; // is FW encrypted or not.
	int efuse_key;	// is Key packed into install.img or not.

	// commands for installing factory
	char ifcmd0[512];
	char ifcmd1[512];

	// for customer_ap
	FILE *customer_fp;
	unsigned int progress;

	// system info
	unsigned int rescue_start;
	unsigned int rescue_size;
	unsigned int norfactory_start;
	unsigned int norfactory_section_size;
	unsigned int norfactory_size;
	unsigned int factory_start;
	unsigned int factory_section_size;
	unsigned int factory_size;
    unsigned int se_storage_Start;
    unsigned int se_storageSize;
    unsigned int se_storage_section_size;
	unsigned int restrom_factory_size;
	unsigned int bootcode_start;
	unsigned int bootcode_size;
	unsigned int norbootcode_start;
	unsigned int norbootcode_size;
	unsigned int bootpart;

	// meminfo
	unsigned int flash_type;
	unsigned int mtd_erasesize;
	unsigned long long flash_size;
	unsigned int page_size;
	unsigned int oob_size;
	unsigned int erased_content;	// for EMMC
	unsigned long long norflash_size;
    unsigned int norflash_mtd_erasesize;
    unsigned int norflash_page_size;
	unsigned int norflash_oob_size;
    unsigned int isNandAndNor;

	// flash layout fo nand
	unsigned int reserved_boot_size;
	unsigned int reserved_boottable_size;
	unsigned int etc_partition_size;
	unsigned int preserve_partition_size;
	unsigned int reserved_remapping_offset;
	unsigned int reserved_remapping_size;
	unsigned int reserved_remapping_sector;
	unsigned int startAddress;
	unsigned int endAddress;

	unsigned int tarfileparsed;

	// for update UI progress
	unsigned long long total_alloc_size;
	char version[16];

	enum eBURN eburn;

	struct t_imgdesc fw[FW_UNKNOWN];
	struct t_imgdesc util[UTIL_MAX];
	struct t_imgdesc cipher_key[KEY_MAX];

	struct t_imgdesc bootloader_tar;
	struct t_imgdesc factory_tar;
	struct t_imgdesc customer;
	struct t_imgdesc video_firmware;
	// new version config framework
	struct t_imgdesc config;
	// compatible old version config framework
	struct t_imgdesc configuration;
	struct t_imgdesc postprocess;
	// ALSADaemon, for UI
	struct t_imgdesc alsadaemon;
	// key verify file
	struct t_imgdesc otp_kevy_verify;
    struct t_imgdesc norbootloader_tar;
    struct t_imgdesc tee_bin;
    struct t_imgdesc bl31_bin;
    struct t_imgdesc xen_bin;
    struct t_imgdesc teeFiles_tar;

	struct t_tarheader* thead_list;
	unsigned int thead_count;

	struct tarinfo tarinfo;
	struct url url;
	char config_buf[CONFIGURATION_MAX_SIZE];
	unsigned int checksum;
	unsigned char sha_hash[32];
	const char *hdd_dev_name;
#ifdef PC_SIMULATE
    char offline_flashSize[16];
    char offline_packageID[16];
#endif  
};
//__attribute__((packed));

extern struct t_PARTDESC *rtk_part_list_sort[NUM_RTKPART];

#ifdef DEBUG_FUNC
void dump_rtk_part_list(struct t_PARTDESC* _rtk_part_list);
#endif
const char* inv_by_fwtype(enum FWTYPE efwtype);
struct t_PARTDESC* find_empty_part(struct t_PARTDESC* _rtk_part_list);
struct t_PARTDESC* find_part_by_part_name(struct t_PARTDESC* _rtk_part_list, const char* part_name);
// for compatibility
struct t_PARTDESC* find_part_by_efwtype(struct t_PARTDESC* _rtk_part_list, enum FWTYPE efwtype);
struct t_FWDESC* find_fw_by_fw_name(const char* fw_name);

int fill_rtkpartdesc_by_dynamicTbl(struct t_rtkimgdesc* prtkimg);
int fill_rtkimgdesc_meminfo(struct t_rtkimgdesc* prtkimg);
int fill_rtkimgdesc_layout(struct t_rtkimgdesc* prtkimg);
int fill_rtkimgdesc_file(struct t_tarheader* pthead, unsigned int tarfile_offset, struct t_rtkimgdesc* prtkimg);
int fill_rtk_part_list_by_config(struct t_rtkimgdesc* prtkimg);

int load_rtkimgdesc(struct t_rtkimgdesc* );
int flush_rtkimgdesc(struct t_rtkimgdesc* );

#ifdef __OFFLINE_GENERATE_BIN__
int prepare_empty_file(char *filename, unsigned long long filesize, unsigned char value);
#endif
#endif
