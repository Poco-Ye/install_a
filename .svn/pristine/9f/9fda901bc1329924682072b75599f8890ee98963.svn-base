#ifndef RTK_COMMON_H
#define RTK_COMMON_H
#include <sys/types.h>

#define uint8_t u8
#define u8 u_int8_t
#define U8 u_int8_t
#define UINT8 u_int8_t

#define uint16_t u16
#define u16 u_int16_t
#define U16 u_int16_t


#define uint32_t u32
#define u32 u_int32_t
#define UINT32 u_int32_t

#define uint64_t u64
#define u64 u_int64_t
#define UINT64 u_int64_t

#ifndef u8
#endif /* u8 */
#ifndef u16
#endif /* u16 */
#ifndef u32
#endif /* u32 */
#ifndef u64
#endif /* u64 */
typedef unsigned int u32;
typedef unsigned char u8;

#include <stdio.h>

#define FLASH_OFFLINE_BIN_WRITER

#define VT100_NONE          "\033[m"
#define VT100_RED           "\033[0;32;31m"
#define VT100_LIGHT_RED     "\033[1;31m"
#define VT100_GREEN         "\033[0;32;32m"
#define VT100_LIGHT_GREEN   "\033[1;32m"
#define VT100_BLUE          "\033[0;32;34m"
#define VT100_LIGHT_BLUE    "\033[1;34m"
#define VT100_DARY_GRAY     "\033[1;30m"
#define VT100_CYAN          "\033[0;36m"
#define VT100_LIGHT_CYAN    "\033[1;36m"
#define VT100_PURPLE        "\033[0;35m"
#define VT100_LIGHT_PURPLE  "\033[1;35m"
#define VT100_BROWN         "\033[0;33m"
#define VT100_YELLOW        "\033[1;33m"
#define VT100_LIGHT_GRAY    "\033[0;37m"
#define VT100_WHITE         "\033[1;37m"

#define FROM_MTD_BLOCK


#define INSTALL_FAIL_LEVEL  0x01UL
#define INSTALL_INFO_LEVEL  0x02UL
#define INSTALL_LOG_LEVEL	0x04UL
#define INSTALL_DEBUG_LEVEL  0x08UL
#define INSTALL_WARNING_LEVEL  0x10UL
#define INSTALL_UI_LEVEL  0x20UL
#define INSTALL_TARLOG_LEVEL  0x100UL
#define INSTALL_MEM_LEVEL  0x200UL

#define install_debug(f, a...) rtk_install_debug_printf(INSTALL_DEBUG_LEVEL, __FILE__, __FUNCTION__, __LINE__, f,##a)
#define install_info(f, a...) rtk_install_debug_printf(INSTALL_INFO_LEVEL, __FILE__, __FUNCTION__, __LINE__, f, ##a)
#define install_log(f, a...) rtk_install_debug_printf(INSTALL_LOG_LEVEL, __FILE__, __FUNCTION__, __LINE__, f, ##a)
#define install_fail(f, a...) rtk_install_debug_printf(INSTALL_FAIL_LEVEL, __FILE__, __FUNCTION__, __LINE__, VT100_LIGHT_RED f VT100_NONE, ##a)
#define install_ui(f, a...) rtk_install_debug_printf(INSTALL_UI_LEVEL, __FILE__, __FUNCTION__, __LINE__, VT100_LIGHT_RED f VT100_NONE, ##a)
#define install_test(f, a...) rtk_install_debug_printf(INSTALL_DEBUG_LEVEL, __FILE__, __FUNCTION__, __LINE__, VT100_LIGHT_RED f VT100_NONE, ##a)
#define install_warn(f, a...) rtk_install_debug_printf(INSTALL_WARNING_LEVEL, __FILE__, __FUNCTION__, __LINE__, VT100_LIGHT_RED f VT100_NONE, ##a)

//get chip id info
#define RTD129x_CHIP_REVISION_A00       (0x00000000) 
#define RTD129x_CHIP_REVISION_A01       (0x00010000)
#define RTD129x_CHIP_REVISION_B00       (0x00020000)
#define REG32( addr )		            (*(volatile UINT32 *)(addr))

#ifdef NAS_ENABLE
/* from mtd/mtd_abi.h */
struct mtd_info_user {
   uint8_t type;
   uint32_t flags;
   uint32_t size;	/* Total size of the MTD */
   uint32_t erasesize;
   uint32_t oobblock;	/* change writesize to oobblock for compatible */
   uint32_t oobsize;	/* Amount of OOB data per block (e.g. 16) */
   uint64_t padding;	/* Old obsolete field; do not use */
};
#else
/* from mtd/mtd_abi.h */
struct mtd_info_user {
   uint8_t type;
   uint32_t flags;
   uint64_t size;   // Total size of the MTD
   uint32_t erasesize;
   uint32_t oobblock;  // Size of OOB blocks (e.g. 512)
   uint32_t oobsize;   // Amount of OOB data per block (e.g. 16)
   uint32_t ecctype;
   uint32_t eccsize;
};
#endif

struct erase_info_user {
   uint32_t start;
   uint32_t length;
};

enum _error_code {
    _eRTK_SUCCESS = 0,
    _eRTK_GENERAL_FAIL = 1,
    _ePARSE_TAR_FAIL,
    _eFILL_RTK_PART_LIST_BY_CONFIG_FAIL,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_BOOTCODE,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_RESCUE,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_FACTORY,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_NORBOOTCODE,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_NORRESCUE,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_NORFACTORY,
    _eFILL_RTKIMGDESC_MEMINFO_FAIL_SE_STORAGE,
    _eFILL_RTK_IMGDESC_LAYOUT_FAIL,
    _eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT,
    _eFILL_RTK_IMGDESC_LAYOUT_FAIL_NORBOOTCODE_EXTRACT,
    _eFILL_RTK_IMGDESC_LAYOUT_FAIL_SIZE_NOT_ENOUGH,
    _eFILL_RTK_IMGDESC_LAYOUT_FAIL_ROOTFS_ETC_ZERO,
    _eRTK_BURN_BOOTCODE_FAIL,
    _eRTK_BURN_NORBOOTCODE_FAIL,
    _eRTK_BURN_FACTORY_FAIL,
    _eRTK_TAG_UPDATE_ETC_FAIL,
    _eRTK_TAG_UPDATE_ETC_FAIL_CONFIG_SIZE,
    _eRTK_TAG_UPDATE_ETC_FAIL_BOOTTABLE_OFFSET,
    _eRTK_TAG_UPDATE_G_NL_FAIL,
    _eRTK_TAG_UPDATE_G_RL_DT_FAIL,
    _eRTK_TAG_UPDATE_G_RL_ROOTFS_FAIL,
    _eRTK_TAG_UPDATE_G_AK_FAIL,
    _eRTK_TAG_UPDATE_G_TEE_FAIL,
    _eRTK_TAG_UPDATE_UBOOT_FAIL,
    _eRTK_TAG_UPDATE_NL_FAIL,
    _eRTK_TAG_UPDATE_NL_DT_FAIL,
    _eRTK_TAG_UPDATE_NL_ROOTFS_FAIL,
    _eRTK_TAG_UPDATE_RL_DT_FAIL,
    _eRTK_TAG_UPDATE_RL_ROOTFS_FAIL,
    _eRTK_TAG_UPDATE_NAS_NL_FAIL,
    _eRTK_TAG_UPDATE_NAS_RL_DT_FAIL,
    _eRTK_TAG_UPDATE_NAS_RL_ROOTFS_FAIL,
    _eRTK_TAG_UPDATE_AK_FAIL,
    _eRTK_TAG_UPDATE_TEEK_FAIL,
    _eRTK_TAG_UPDATE_BL31_FAIL,
    _eRTK_TAG_UPDATE_XEN_FAIL,
    _eRTK_TAG_UPDATE_VK_FAIL,
    _eRTK_TAG_UPDATE_VK2_FAIL,
    _eRTK_TAG_UPDATE_DALOGO_FAIL,
    _eRTK_TAG_UPDATE_DVLOGO_FAIL,
    _eRTK_TAG_UPDATE_PARTITION_FAIL,
    _eRTK_TAG_SECURE_FAIL,
    _eUNKOWN_ERROR_CODE
};

struct _error_t {
   enum _error_code error_code;
   char _error_msg_short[128]; //error message for end user
   char _error_msg_detail[256]; //detail error message
};

const struct _error_t _error_info[_eUNKOWN_ERROR_CODE+1] = {
    { _eRTK_SUCCESS                              ,"Success!"                        ,"Success!" },
    { _eRTK_GENERAL_FAIL                         ,"General error!"                  ,"General error!" },
    { _ePARSE_TAR_FAIL                           ,"Extract file error!"             ,"Extract file fail!" }, //parse_tar()
    { _eFILL_RTK_PART_LIST_BY_CONFIG_FAIL        ,"Load configuration error!"       ,"Load configuration fail" }, //fill_rtk_part_list_by_config()
    { _eFILL_RTKIMGDESC_MEMINFO_FAIL             ,"Load memory info error!"         ,"Load memory info fail" }, //fill_rtkimgdesc_meminfo()
    { _eFILL_RTKIMGDESC_MEMINFO_FAIL_BOOTCODE                    ,"Load memory info error!"         ,"bootcode_start or bootcode_size parameter error!" }, //fill_rtkimgdesc_meminfo()
    { _eFILL_RTKIMGDESC_MEMINFO_FAIL_RESCUE                      ,"Load memory info error!"         ,"rescue_start or rescue_size parameter error!" }, //fill_rtkimgdesc_meminfo()
    { _eFILL_RTKIMGDESC_MEMINFO_FAIL_FACTORY                     ,"Load memory info error!"         ,"factory_start or factory_size parameter error!" }, //fill_rtkimgdesc_meminfo()
    { _eFILL_RTKIMGDESC_MEMINFO_FAIL_NORFACTORY                  ,"Load memory info error!"         ,"norfactory_start or norfactory_size parameter error!" }, //fill_rtkimgdesc_meminfo()
    { _eFILL_RTK_IMGDESC_LAYOUT_FAIL             ,"Image layout error!"             ,"Could be bootloader.tar extract error, flash space not enough or rootfs etc partition is 0." }, //fill_rtkimgdesc_layout()
    { _eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT            ,"Image layout error!"             ,"bootloader.tar extract error." }, //fill_rtkimgdesc_layout()
    { _eFILL_RTK_IMGDESC_LAYOUT_FAIL_NORBOOTCODE_EXTRACT         ,"Image layout error!"             ,"norbootloader.tar extract error." }, //fill_rtkimgdesc_layout()
    { _eFILL_RTK_IMGDESC_LAYOUT_FAIL_SIZE_NOT_ENOUGH             ,"Image layout error!"             ,"Flash space not enough." }, //fill_rtkimgdesc_layout()
    { _eFILL_RTK_IMGDESC_LAYOUT_FAIL_ROOTFS_ETC_ZERO             ,"Image layout error!"             ,"rootfs or etc partition is 0." }, //fill_rtkimgdesc_layout()
    { _eRTK_BURN_BOOTCODE_FAIL               ,"Install bootloader error!"       ,"Extract bootloader related file error or program bootloader error." }, //rtk_burn_bootcode_mac()
    { _eRTK_BURN_NORBOOTCODE_FAIL            ,"Install norbootloader error!"    ,"Extract norbootloader related file error or program norbootloader error." }, //rtk_burn_bootcode_mac()
    { _eRTK_BURN_FACTORY_FAIL                ,"Install factory error"           ,"Could be factory.tar extract error or file missing in factory." }, //rtk_burn_factory_mac()
    { _eRTK_TAG_UPDATE_ETC_FAIL                  ,"Update user data error"          ,"Update etc partition fail ." },
    { _eRTK_TAG_UPDATE_ETC_FAIL_CONFIG_SIZE                      ,"Update user data error"          ,"update_etc not set, but the etc configuration size is not the same in flash." },
    { _eRTK_TAG_UPDATE_ETC_FAIL_BOOTTABLE_OFFSET                 ,"Update user data error"          ,"update_etc not set, but the offset is not the same in flash." },
    { _eRTK_TAG_UPDATE_NL_FAIL                   ,"Update OS error"                 ,"Update normal linux fail." },
    { _eRTK_TAG_UPDATE_NL_DT_FAIL                ,"Update OS device tree error" 	,"Update normal linux device tree fail." },
    { _eRTK_TAG_UPDATE_NL_ROOTFS_FAIL            ,"Update file system error"        ,"Update normal linux rootfs fail." },
    { _eRTK_TAG_UPDATE_RL_DT_FAIL                ,"Update rescue device tree error"		,"Update rescue linux device tree fail." },
    { _eRTK_TAG_UPDATE_RL_ROOTFS_FAIL            ,"Update rescue file system error"     ,"Update rescue linux rootfs fail." },
    { _eRTK_TAG_UPDATE_NAS_NL_FAIL                   ,"Update redundant kernel error"                 ,"Update NAS rescue linux fail." },
    { _eRTK_TAG_UPDATE_NAS_RL_DT_FAIL                ,"Update redundant rescue DTB error"		,"Update NAS rescue linux DTB fail." },
    { _eRTK_TAG_UPDATE_NAS_RL_ROOTFS_FAIL            ,"Update redundant rescue rootfs error"     ,"Update NAS rescue linux rootfs fail." },
    { _eRTK_TAG_UPDATE_AK_FAIL                   ,"Update audio fw error"           ,"Update audio fw fail." },
    { _eRTK_TAG_UPDATE_TEEK_FAIL                   ,"Update TEE fw error"           ,"Update TEE fw fail." },
    { _eRTK_TAG_UPDATE_VK_FAIL                   ,"Update video fw error"           ,"Update video fw fail." },
    { _eRTK_TAG_UPDATE_DALOGO_FAIL               ,"Update default audio logo error" ,"Update default audio logo fail." },
    { _eRTK_TAG_UPDATE_DVLOGO_FAIL               ,"Update default video logo error" ,"Update default video logo fail." },
    { _eRTK_TAG_UPDATE_PARTITION_FAIL            ,"Update Partition Error"          ,"Update Partition Error." },
    { _eRTK_TAG_SECURE_FAIL                      ,"Secure check Error"              ,"Secure check Error" },
    { _eUNKOWN_ERROR_CODE                        ,"Unknown error code!"             ,"Unknown error code!" },
};





#define ARRAY_COUNT(x)  (sizeof(x) / sizeof((x)[0]))
#define FILESIZELEN 11

unsigned long long fd_to_fd(int sfd, int dfd, unsigned long long length, unsigned int* pchecksum=NULL, FILE* ffd=NULL, char* filename=NULL, FILE* customer_fp=NULL);
int rtk_extract_file(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, const char* file_path);
unsigned int octalStringToInt(const char* str, unsigned int lenstr);
int rtk_get_size_of_file(const char* file_path, unsigned int *file_size);
u32 get_checksum(u8 *p, u32 len);
int rtk_command(const char* command, int line, const char* file, int show=1);
int rtk_file_to_string(const char* path, char* str);
int rtk_file_to_flash(const char* filepath, unsigned long long soffset, const char* devpath, unsigned long long doffset, unsigned long long len, unsigned int* checksum);
int rtk_ptr_to_file(const char* filepath, unsigned int soffset, void* ptr, unsigned int len);
int rtk_ptr_to_flash(void* ptr, unsigned int len, const char* devpath, unsigned int doffset);
int rtk_flash_to_ptr(const char* devpath, unsigned int soffset, void* ptr, unsigned int len);
int rtk_file_to_ptr(const char* filepath, unsigned int soffset, void* ptr, unsigned int len);
int rtk_file_to_mtd(const char* filepath, unsigned int soffset, unsigned int imglen, const struct t_rtkimgdesc* prtkimgdesc, unsigned int doffset, unsigned int alignlen, unsigned int* checksum);
int rtk_erase_mtd(struct t_rtkimgdesc* prtkimgdesc, unsigned int erase_start, unsigned int erase_length);
int rtk_unlock_mtd(struct t_rtkimgdesc* prtkimgdesc, unsigned int unlock_start, unsigned int unlock_length);
int rtk_file_verify(const char* sfilepath, unsigned long long soffset, const char* dfilepath, unsigned long long doffset, unsigned int imglen, unsigned int *err_count, unsigned int *checksum=NULL);
int rtk_file_checksum(const char* dfilepath, unsigned int doffset, unsigned int imglen, unsigned char *checksum);
int rtk_file_checksum_pc(const char* dfilepath, const char* imgpath, unsigned int imglen, unsigned char *checksum);
int rtk_ptr_verify(const char* sfilepath, unsigned int soffset, const char* dmemory, unsigned int doffset, unsigned int imglen, unsigned int *err_count);
int rtk_find_file_in_dir(const char *pdirpath, const char *pkeyword, char *filename, const int filename_maxlen);
int rtk_find_dir_path(char *path, char *dirpath, int len);
int rtk_dump_flash(struct t_rtkimgdesc* prtkimgdesc);
void rtk_install_debug_printf(u32 debugLevel, const char* filename, const char* funcname, u32 fileline, const char* fmt, ...);
void rtk_hexdump( const char * str, unsigned char * pcBuf, unsigned int length );
unsigned int get_chip_rev_id();
int read_key(const char *filename, unsigned char* key, int size);

#ifdef __OFFLINE_GENERATE_BIN__
int rtk_file_to_virt_nand_with_ecc(const char* filepath, unsigned int soffset, unsigned int imglen, const struct t_rtkimgdesc* prtkimgdesc, unsigned int doffset, unsigned char block_indicator, unsigned int* checksum, unsigned int mode = 0);
int rtk_yaffs_to_virt_nand_with_ecc(const char* filepath, unsigned int soffset, unsigned int imglen, const struct t_rtkimgdesc* prtkimgdesc, unsigned int doffset, unsigned char block_indicator, unsigned int* checksum);
#endif
#endif
