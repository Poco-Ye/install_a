#ifndef RTKINSTALL_H
#define RTKINSTALL_H
#include <sys/types.h>
#include <stdlib.h>
#include <rtk_common.h>

#define NAND_BOOTCODE_RESERVED_SIZE    0x1000000
inline unsigned long long SIZE_ALIGN_BOUNDARY_LESS(unsigned long long len, unsigned long long size)
{
	return ((len) & ~((size) - 1));
}

inline unsigned long long SIZE_ALIGN_BOUNDARY_MORE(unsigned long long len, unsigned long long size)
{
	return (((len - 1) & ~((size) - 1)) + size);
}

#define be32_to_cpu(x)  do {x = __swap32((x));} while(0)
#define cpu_to_be32(x)  do {x = __swap32((x));} while(0)

static inline u32 __swap32(u32 val) {
    return ((val & 0x000000FF) << 24) |
        ((val & 0x0000FF00) << 8) |
        ((val & 0x00FF0000) >> 8) |
        ((val & 0xFF000000) >> 24);
}

#define get_be32_to_cpu(x)  __swap32((x))

#define UPGRAD_SIG "UPGRAD__"
typedef struct {
   u8   signature[8];
   u32  checksum;
   u8   version;
   u8   reserved[7];
   u32  paddings;
   u32  part_list_len;
   u32  fw_list_len;
} __attribute__((packed)) fw_desc_table_t;

typedef struct {
   u8   type;
#if BYTE_ORDER == LITTLE_ENDIAN
   u8   reserved:7,
        ro:1;
#elif BYTE_ORDER == BIG_ENDIAN
   u8   ro:1,
        reserved:7;
#else
#error "Please fix <asm/byteorder.h>"
#endif
   u64  length;
   u8   fw_count;
   u8   fw_type;
#ifdef EMMC_SUPPORT
   u8   emmc_partIdx;
   u8   reserved_1[3];
#else
   u8   reserved_1[4];
#endif
   u8   mount_point[32];
} __attribute__((packed)) part_desc_entry_v2_t;

typedef enum {
   PART_TYPE_RESERVED = 0,
   PART_TYPE_FW,
   PART_TYPE_FS,
} part_type_code_t;

typedef struct {
   u8   type;
#if BYTE_ORDER == LITTLE_ENDIAN
   u8   reserved:6,
        lzma:1,
        ro:1;
#elif BYTE_ORDER == BIG_ENDIAN
   u8   ro:1,
        lzma:1,
        reserved:6;
#else
#error "Please fix <asm/byteorder.h>"
#endif
   u32  version;
   u32  target_addr;
   u64  offset;
   u32  length;
   u32  paddings;
   u8   sha_hash[32];
   u8   reserved_1[6];
} __attribute__((packed)) fw_desc_entry_v2_t;

typedef struct {
    fw_desc_entry_v2_t v2;
    u32 act_size;
    u8  hash[32];
    u8  part_num;
    u8  reserved[27];
} __attribute__((packed)) fw_desc_entry_v12_t;

typedef struct {
    fw_desc_entry_v2_t v2;
    u32 act_size;
    u8  part_num;
    u8  RSA_sign[256];
    u8  reserved[27];
} __attribute__((packed)) fw_desc_entry_v22_t;

struct rtk_fw_header
{
   fw_desc_table_t fw_tab;
   part_desc_entry_v2_t part_desc[10];
   fw_desc_entry_v22_t fw_desc[30];
   unsigned int fw_intex[15];

   unsigned int kernel_index;
   unsigned int audio_index;
   unsigned int video_index;
   unsigned int rootfs_index;
   unsigned int resource_index;

   unsigned int secure_boot;
   unsigned int part_count;
   unsigned int fw_count;
   unsigned int valid;
};

typedef enum {
   FW_TYPE_RESERVED = 0,
   FW_TYPE_BOOTCODE,
   FW_TYPE_KERNEL,
   FW_TYPE_RESCUE_DT,
   FW_TYPE_KERNEL_DT,
   FW_TYPE_RESCUE_ROOTFS, //5
   FW_TYPE_KERNEL_ROOTFS,
   FW_TYPE_AUDIO,
   FW_TYPE_AUDIO_FILE,
   FW_TYPE_VIDEO_FILE,
   FW_TYPE_EXT4, //10
   FW_TYPE_UBIFS,
   FW_TYPE_SQUASH,
   FW_TYPE_EXT3,
   FW_TYPE_ODD,
   FW_TYPE_YAFFS2, //15
   FW_TYPE_ISO,
   FW_TYPE_SWAP,
   FW_TYPE_NTFS,
   FW_TYPE_JFFS2,
   FW_TYPE_IMAGE_FILE, //20
   FW_TYPE_IMAGE_FILE1,
   FW_TYPE_IMAGE_FILE2,
   FW_TYPE_AUDIO_FILE1,
   FW_TYPE_AUDIO_FILE2,
   FW_TYPE_VIDEO_FILE1, //25
   FW_TYPE_VIDEO_FILE2,
   FW_TYPE_VIDEO,
   FW_TYPE_VIDEO2,
   FW_TYPE_ECPU,
   FW_TYPE_TEE, //30
   FW_TYPE_GOLD_KERNEL,
   FW_TYPE_GOLD_RESCUE_DT,
   FW_TYPE_GOLD_RESCUE_ROOTFS,
   FW_TYPE_GOLD_AUDIO,
   FW_TYPE_GOLD_TEE, //35
   FW_TYPE_CONFIG,
   FW_TYPE_UBOOT,
   FW_TYPE_BL31,
   FW_TYPE_XEN,
   FW_TYPE_GOLD_BL31,
   FW_TYPE_RSA_KEY_FW,
   FW_TYPE_RSA_KEY_TEE,
   FW_TYPE_UNKNOWN
} fw_type_code_t;

#define VERONA_INSTALLER_SIGNATURE		"Verona Installation Packages"
#define FIRMWARE_DESCRIPTION_TABLE_SIGNATURE	"VERONA__"	//Max 8 bytes length.
//for old fw_table define
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_01	0x01		//1 byte table version.
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_11	0x11		//1 byte table version.
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_21	0x21		//1 byte table version.
//for new fw_table define
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_02	0x02		//1 byte table version.
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_12	0x12		//1 byte table version.
#define FIRMWARE_DESCRIPTION_TABLE_VERSION_22	0x22		//1 byte table version.

#define FW_IS_SET(x, y) ((x)->valid & ((unsigned int)0x01)<<(y))
#define FW_SET(x, y) ((x)->valid = (x)->valid | ((unsigned int)0x01)<<(y))
#define FW_CLEAR_ALL(x) ((x)->valid = 0)
int load_fwdesc(struct rtk_fw_header* , unsigned int _NAND_BOOTCODE_RESERVED_SIZE=NAND_BOOTCODE_RESERVED_SIZE);
int save_fwdesc(struct rtk_fw_header* , struct t_rtkimgdesc*);
#ifdef EMMC_SUPPORT
int save_fwdesc_emmc(struct rtk_fw_header*, struct t_rtkimgdesc*);
#endif
int load_fwdesc_nor(struct rtk_fw_header* );
#if defined(NAS_ENABLE) && defined(CONFIG_BOOT_FROM_SPI)
int save_fwdesc_nor(struct rtk_fw_header* fw, struct t_rtkimgdesc* prtkimg);
#else
int save_fwdesc_nor(struct rtk_fw_header* fw);
#endif
int copy_fw_from_local_to_file(struct t_rtkimgdesc* prtkimgdesc, fw_type_code_t efwtype, const char* filename);
int copy_videofw_from_local_to_file(struct t_rtkimgdesc* prtkimgdesc, const char* filename);
int findNandBackupAreaStartEndAddress(unsigned int* startAddress, unsigned int* endAddress, unsigned int _NAND_BOOTCODE_RESERVED_SIZE=NAND_BOOTCODE_RESERVED_SIZE);
int check_boot_code_size(unsigned int *nand_boot_size, unsigned int *factory_size);
// not used
int findNandReservedSpaceStartEndAddress(unsigned int* startAddress, unsigned int* endAddress);
#endif
