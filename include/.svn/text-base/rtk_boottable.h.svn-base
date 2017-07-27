#ifndef RTK_BOOTTABLE_H
#define RTK_BOOTTABLE_H

struct t_imgdesc;

enum E_FWTYPE
{
   FWTYPE_KERNEL=0,  // Normal Kernel
   FWTYPE_RESCUE_DT, // Rescue Kernel device tree
   FWTYPE_KERNEL_DT, // Normal Kernel device tree
   FWTYPE_RESCUE_ROOTFS, // Rescue root file system
   FWTYPE_KERNEL_ROOTFS, // Normal root file system
   FWTYPE_NAS_RESCUE_KERNEL,  // NAS rescue Kernel
   FWTYPE_NAS_RESCUE_DT, // NAS Rescue Kernel DTB
   FWTYPE_NAS_RESCUE_ROOTFS, // NAS Rescue root file system
   FWTYPE_AKERNEL,   // Audio Kernel
   FWTYPE_TEEKERNEL,   // TEE Kernel
   FWTYPE_VKERNEL,      // Video Kernel
   FWTYPE_VKERNEL2,      // Video2 Kernel
   FWTYPE_ECPU,			 // 8051 Kernel
   FWTYPE_DALOGO,    // Default LOGO
   FWTYPE_DILOGO,    // Default LOGO(Image)
   FWTYPE_DVLOGO,    // Default LOGO(Video)
   FWTYPE_CALOGO1,    // Customer LOGO1
   FWTYPE_CILOGO1,
   FWTYPE_CVLOGO1,    // Customer LOGO1
   FWTYPE_CALOGO2,    // Customer LOGO2
   FWTYPE_CILOGO2,
   FWTYPE_CVLOGO2,    // Customer LOGO2
   FWTYPE_NRESCUE,      // Native Rescue
   FWTYPE_RESCUE,    // Rescue
   FWTYPE_SROOTFS,      // Small rootfs
   FWTYPE_FWTBL,	// fw table
   FWTYPE_GOLD_KERNEL,  // Golden rescue Kernel
   FWTYPE_GOLD_RESCUE_DT, // Golden Rescue Kernel DTB
   FWTYPE_GOLD_RESCUE_ROOTFS, // Golden Rescue root file system
   FWTYPE_GOLD_AKERNEL,   //Golden Audio Kernel
   FWTYPE_GOLD_TEEKERNEL,   //Golden TEE Kernel
   FWTYPE_BL31,
   FWTYPE_XEN,       //XEN_OS for Hypervisor mode
   FWTYPE_P_UBOOT,   // Uboot partition
   FWTYPE_END
};

struct S_IMGLOCATION
{
   unsigned long long offset;
   unsigned long long size;
   unsigned int target;
   char type[16];
   // maybe we have more info in the future, ex:checksum
};

struct S_IMGDESC
{
    char imgname[64];
    struct S_IMGLOCATION loc;
};

struct S_IMGLIST
{
   unsigned int fwcount;
   struct S_IMGDESC list[FWTYPE_END];
};

enum E_FS
{
   FS_SQUASH,
   FS_JFFS2,
   FS_YAFFS2,
   FS_RAWFILE,
   FS_EXT4,
   FS_UBIFS,
   FS_END
};
struct S_PARTDESC
{
   char imgname[64];
   char partname[32];
   char mount_point[64];
   char mount_dev[64];
   char type[16];
   enum E_FS efs;
   struct S_IMGLOCATION loc;
};

struct S_PARTLIST
{
   unsigned int partcount;
   struct S_PARTDESC list[20];
};

struct S_MBRDESC
{
   char imgname[64];
   unsigned int size;
   unsigned long long offset;
};

struct S_MBRLIST
{
   unsigned int mbrcount;
   struct S_MBRDESC list[20];
};

enum E_TAG
{
   TAG_UNKNOWN = 0,
   TAG_BOOTCODE,
   TAG_FACTORY,
   TAG_AQ,
   TAG_PQ,
   TAG_APK,
   TAG_UPDATE_ETC,
   TAG_UPDATE_GOLD_NL,
   TAG_UPDATE_GOLD_RL_DT,
   TAG_UPDATE_GOLD_RL_ROOTFS,
   TAG_UPDATE_GOLD_AK,
   TAG_UPDATE_GOLD_TEE,
   TAG_UPDATE_NL,
   TAG_UPDATE_NL_DT,
   TAG_UPDATE_NL_ROOTFS,
   TAG_UPDATE_RL_DT,
   TAG_UPDATE_RL_ROOTFS,
   TAG_UPDATE_NAS_NL,
   TAG_UPDATE_NAS_RL_DT,
   TAG_UPDATE_NAS_RL_ROOTFS,
   TAG_UPDATE_AK,
   TAG_UPDATE_TEE,
   TAG_UPDATE_BL31,
   TAG_UPDATE_XEN,
   TAG_UPDATE_VK,
   TAG_UPDATE_VK2,
   TAG_UPDATE_ECPUK,
   TAG_UPDATE_DALOGO,
   TAG_UPDATE_DILOGO,
   TAG_UPDATE_DVLOGO,
   TAG_UPDATE_ROOTFS,
   TAG_UPDATE_RES,
   TAG_UPDATE_SYSTEM,
   TAG_UPDATE_CACHE,
   TAG_UPDATE_DATA,
   TAG_UPDATE_UBOOT,
   TAG_UPDATE_OTHER_PARTITION1,
   TAG_UPDATE_OTHER_PARTITION2,
   TAG_UPDATE_OTHER_PARTITION3,
   TAG_UPDATE_OTHER_PARTITION4,
   TAG_UPDATE_OTHER_PARTITION5,
   TAG_UPDATE_OTHER_PARTITION6,
   TAG_UPDATE_OTHER_PARTITION7,
   TAG_UPDATE_OTHER_PARTITION8,
   TAG_UPDATE_OTHERS,
   TAG_COMPLETE,
};

const char TAG_PART_NAME[][64] = {
   "Unknown Data",
   "FW Image (BOOT)",         // BOOTCODE
   "FW Image (FACTORY)",    // FACTORY
   "AQ Data",    // AQ
   "PQ Data",    // PQ
   "Application APKs",    // APK
   "FW Image (ETC)",
   "FW Image (LINUX)",
   "FW Image (LINUX_DT)",
   "FW Image (LINUX_ROOTFS)",
   "FW Image (RESCUE_DT)",
   "FW Image (RESCUE_ROOTFS)",
   "FW Image (AUDIO)",
   "FW Image (VIDEO)",
   "FW Image (VIDEO2)",
   "FW Image (ECPU)",
   "FW Image (DALOGO)",
   "FW Image (DVLOGO)",
   "FW Image (RES)",
   "FW Image (SYSTEM)",
   "FW Image (CACHE)",
   "FW Image (DATA)",
   "FW Image (PART1)",
   "FW Image (PART2)",
   "FW Image (PART3)",
   "FW Image (PART4)",
   "FW Image (PART5)",
   "FW Image (PART6)",
   "FW Image (PART7)",
   "FW Image (PART8)",
   "FW Image (OTHERS)",
   "COMPLETE"
};

enum E_BOOTTYPE
{
   BOOTTYPE_UNKNOWN_BOOTTYPE=0,
   BOOTTYPE_NL_BIG_ROOTFS,
   BOOTTYPE_NL_SMALL_ROOTFS,
   BOOTTYPE_TMP_RESCUE,
   BOOTTYPE_NATIVE_RESCUE,
   BOOTTYPE_COMPLETE,
   BOOTTYPE_END
};
struct S_BOOTTABLE
{
   char date[16];
   char time[16];
   E_BOOTTYPE boottype;
   E_TAG tag;
   char imgcksum[36];
   char url[256];
   int ssu_work_part;
   struct S_IMGLIST fw;
   struct S_PARTLIST part;
#ifdef EMMC_SUPPORT
   struct S_MBRLIST mbr;
   int mbr_matched;
   unsigned int bootpart;
#endif
};

#if defined(EMMC_SUPPORT) || defined(NAS_ENABLE)
struct S_MBR_PARTITION_ENTRY
{
	unsigned char active;
	unsigned char head_start;
	unsigned char sector_start;
	unsigned char cylinder_start;
	unsigned char id;
	unsigned char head_end;
    unsigned char sector_end;
    unsigned char cylinder_end;
	unsigned int rel_sector_start;
	unsigned int sector_num;
};

struct __attribute__((packed)) S_MBR_TABLE
{
	unsigned char codes[446];
	struct S_MBR_PARTITION_ENTRY part[4];
	unsigned short signature;
};
#endif
#ifdef NAS_ENABLE
#define RT_NAS_MAGIC            "0bda"      /* Realtek NAS disk magic key   */
#define RT_NAS_MAGIC_HEX        0x0bda
#define DOS_DISK_SIG_OFFSET     0x1b8
#endif
const char* inv_boottype(enum E_BOOTTYPE boottype);
const char* inv_fwtype(enum E_FWTYPE fwtype);
const char* inv_efs(enum E_FS efs);
enum E_FS string_inv_to_efs(const char* str);
enum E_FWTYPE string_inv_to_fwtype(const char* str);
enum E_BOOTTYPE string_inv_to_boottype(const char* str);


#ifdef EMMC_SUPPORT
S_BOOTTABLE* read_boottable_emmc(S_BOOTTABLE*, struct t_rtkimgdesc*);
int write_boottable_emmc(S_BOOTTABLE* , unsigned int, unsigned int, struct t_rtkimgdesc* prtkimg);
int rtk_mount_format_emmc(S_BOOTTABLE* pboottable, struct t_rtkimgdesc* prtkimg);
#endif
S_BOOTTABLE* read_boottable(S_BOOTTABLE*, struct t_rtkimgdesc*);
S_BOOTTABLE* write_boottable(S_BOOTTABLE* , unsigned int, unsigned int, struct t_rtkimgdesc* prtkimg, bool bFlush=true);
#ifdef NAS_ENABLE
int get_hdd_block_name(const char** ppstr);
S_BOOTTABLE* read_boottable_hdd(S_BOOTTABLE*, struct t_rtkimgdesc*);
int write_boottable_hdd(S_BOOTTABLE* , const char*, struct t_rtkimgdesc*);
#endif

char* hash_file(const char* img_path);
const char* inv_boottype(enum E_BOOTTYPE boottype);

// BOOTTABLE opertation
int get_index_by_partname(S_BOOTTABLE* pboottable, const char* partname);
int remove_part_by_partname(S_BOOTTABLE* pboottable, const char* partname);
int update_ssu_work_part(S_BOOTTABLE* pboottable, int next_ssu_work_part);
int add_part(S_BOOTTABLE* pboottable, const char* partname, const char* mount_point, const char* mount_dev, E_FS efs, unsigned long long offset, unsigned long long size, const char *compress_type=NULL);
int add_part(S_BOOTTABLE* pboottable, t_imgdesc *pimgdesc);
int update_fw(S_BOOTTABLE* pboottable, E_FWTYPE efwtype, unsigned int mem_offset, unsigned long long flash_offset, unsigned int img_size, const char *compress_type=NULL);
int update_fw(S_BOOTTABLE* pboottable, E_FWTYPE efwtype,  t_imgdesc *pimgdesc);
#ifdef EMMC_SUPPORT
int add_mbr(S_BOOTTABLE* pboottable, char *imgname, unsigned int blkoffset, unsigned int imgsize);
#endif
#endif
