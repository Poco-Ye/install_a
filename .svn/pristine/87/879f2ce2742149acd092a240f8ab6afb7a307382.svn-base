#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <rtk_common.h>
#include <rtk_burn.h>
#include <rtk_config.h>
#include <rtk_def.h>

extern u32 gDebugPrintfLogLevel;
extern struct t_DYNAMIC_PARTDESC dynamic_part_tbl[NUM_SUPPORT_DYNAMIC_PART_TBL];
extern struct t_PARTDESC rtk_part_list[NUM_RTKPART];

static inline char* skip_char(char* str, char c)
{
   do
   {
      if(*str==0) return NULL;
      if(*str==c) return str+1;
      str++;
   } while(1);
   return NULL;
}

static inline char* skip_space(char* str)
{
   do
   {
      if(*str==0) return NULL;
      if(*str != ' ') return str;
      str++;
   } while(1);
   return NULL;
}

static inline int skip_quotation(char *str, char **string, int *string_len)
{
   char *find_first = NULL, *find_last = NULL;

   find_first = strchr(str, '\"');
   find_last = strrchr(str, '\"');

   if (find_first != NULL && find_last != NULL && find_first != find_last) {
      *string = find_first + 1;
      *string_len = find_last - find_first - 1;
      return 0;
   }
   else {
      *string = str;
      *string_len = strlen(str);
      return -1;
   }
}

static inline int del_cr_lf(char* str)
{
   char *find_cr_last = NULL;
   char *find_lf_last = NULL;

   do
   {
      find_cr_last = strrchr(str, '\r');
      find_lf_last = strrchr(str, '\n');

      if (find_cr_last == NULL && find_lf_last == NULL) {
         return 0;
      }
      if (find_cr_last != NULL){
         *find_cr_last = '\0';
      }
      if (find_lf_last != NULL){
         *find_lf_last = '\0';
      }
   } while(1);
}

static struct {
    E_TAG etag;
    FWTYPE fwtype;
} eTag_2_fwType[] =
{
    {TAG_BOOTCODE,         FW_BOOTCODE},
    {TAG_FACTORY,          FW_FACTORY},
    {TAG_UPDATE_ETC,       FW_USR_LOCAL_ETC},
    {TAG_UPDATE_NL,        FW_KERNEL},
    {TAG_UPDATE_NL_DT,     FW_KERNEL_DT},
    {TAG_UPDATE_NL_ROOTFS, FW_KERNEL_ROOTFS},
    {TAG_UPDATE_RL_DT,     FW_RESCUE_DT},
    {TAG_UPDATE_RL_ROOTFS, FW_RESCUE_ROOTFS},
    {TAG_UPDATE_NAS_NL,    FW_NAS_KERNEL},
    {TAG_UPDATE_NAS_RL_DT, FW_NAS_RESCUE_DT},
    {TAG_UPDATE_NAS_RL_ROOTFS, FW_NAS_RESCUE_ROOTFS},
    {TAG_UPDATE_AK,        FW_AUDIO},
    {TAG_UPDATE_TEE,       FW_TEE},
    {TAG_UPDATE_BL31,      FW_BL31},
    {TAG_UPDATE_XEN,       FW_XEN},
    {TAG_UPDATE_DALOGO,    FW_AUDIO_BOOTFILE},
    {TAG_UPDATE_DILOGO,    FW_IMAGE_BOOTFILE},
    {TAG_UPDATE_DVLOGO,    FW_VIDEO_BOOTFILE},
    {TAG_UPDATE_ROOTFS,    FW_ROOTFS},
    {TAG_UPDATE_RES,       FW_RES},
    {TAG_UPDATE_SYSTEM,    FW_SYSTEM},
    {TAG_UPDATE_CACHE,     FW_CACHE},
    {TAG_UPDATE_DATA,      FW_DATA}
};

FWTYPE etag_to_fwtype(E_TAG tag)
{
	unsigned int i;
	for( i=0; i<(sizeof(eTag_2_fwType)/sizeof(eTag_2_fwType[0])); i++ ) {
		if( eTag_2_fwType[i].etag == tag )
			return eTag_2_fwType[i].fwtype;
	}

	install_info("unknown E_TAG(%d)\n", tag);
	return FW_UNKNOWN;
}

const char* inv_efs_to_str(enum eFS efs_type)
{
   switch(efs_type) {
      case FS_TYPE_UBIFS:
         return "ubifs";
      case FS_TYPE_SQUASH:
         return "squashfs";
      case FS_TYPE_YAFFS2:
         return "yaffs2";
      case FS_TYPE_JFFS2:
         return "jffs2";
      case FS_TYPE_RAWFILE:
         return "rawfile";
      case FS_TYPE_NONE:
         return "none";
#ifdef EMMC_SUPPORT
	  case FS_TYPE_EXT4:
		 return "ext4";
#endif
      default:
         return "Unknown";
   }

}

static enum eFS inv_efs_by_str(const char* str)
{
	if ( 0 == strcmp(str, "squash") ) {
		return FS_TYPE_SQUASH;
	} else if ( 0 == strcmp(str, "ubifs") ) {
		return FS_TYPE_UBIFS;
	} else if ( 0 == strcmp(str, "squashfs") ) {
		return FS_TYPE_SQUASH;
	} else if ( 0 == strcmp(str, "yaffs2") ) {
		return FS_TYPE_YAFFS2;
	} else if ( 0 == strcmp(str, "jffs2") ) {
		return FS_TYPE_JFFS2;
	} else if ( 0 == strcmp(str, "rawfile") ) {
		return FS_TYPE_RAWFILE;
	} else if ( 0 == strcmp(str, "ext4") ) {
		return FS_TYPE_EXT4;
	} else if ( 0 == strcmp(str, "none") ) {
		return FS_TYPE_NONE;
	}

	return FS_TYPE_UNKOWN;
}

int add_firmware(char* str, struct t_rtkimgdesc* prtkimg)
{
	struct t_FWDESC* rtk_fw;
	char firmware_name[128], filename[128];
	unsigned int target;

	// sanity-check
	if(str == NULL||*str==0)
		return -1;

	if(str[0] == ' ')
		str = skip_space(str);
	// firmware_name filename target
	sscanf(str, "%s %s %x", firmware_name, filename, &target);

	rtk_fw = find_fw_by_fw_name(firmware_name);

	// sanity-check
	if(rtk_fw == NULL) {
		install_debug("Can't get firmare(%s)\r\n", str);
		return -1;
	}

	// show info
	install_info("Add firmware(%s), filename(%s), target(0x%08x)\r\n", firmware_name, filename, target);

	// fill rtk_fw
	sprintf(rtk_fw->firmware_name, "%s", firmware_name);
	sprintf(rtk_fw->filename, "%s", filename);
	rtk_fw->target = target;

	return 0;
}

struct t_PARTDESC* add_dynamic_tblInfo(char* str)
{
	int i, firstEmptyIdx=-1, targetIdx=-1;
	unsigned int flash_targetSizeM = 0;
	// sanity-check
	if(str == NULL||*str==0)
		return NULL;

	if(str[0] == ' ')
		str = skip_space(str);

	sscanf(str, "%u", &flash_targetSizeM);

	//Decide inserted index
	for(i=0; i< NUM_SUPPORT_DYNAMIC_PART_TBL; i++) {
		if (dynamic_part_tbl[i].flash_sizeM == flash_targetSizeM) {
			targetIdx = i;
			break;
		}
		if (dynamic_part_tbl[i].flash_sizeM == 0  && firstEmptyIdx == -1) {
			firstEmptyIdx = i;
		}
	}
	if (targetIdx != i) {
		targetIdx = firstEmptyIdx;
	}
	install_log("Insert target_size:%u into %d th table\n\r", flash_targetSizeM, targetIdx);

	dynamic_part_tbl[targetIdx].flash_sizeM = flash_targetSizeM;

	return (struct t_PARTDESC*)&(dynamic_part_tbl[targetIdx].partDescTbl[0]);
}

int add_partition(char* str, struct t_rtkimgdesc* prtkimg, struct t_PARTDESC* _pTargetPartDesc)
{
   static int id=0;

   struct t_PARTDESC* rtk_part;
   char partition_name[32], mount_point[32], filesystem[32], filename[32], mount_dev[32];
   unsigned long long min_size;
   unsigned int DEFAULT_YAFFS_SIZE;
#ifdef ENABLE_ERASE_CHECK
    unsigned int bErase;
#endif
   DEFAULT_YAFFS_SIZE = 6*prtkimg->mtd_erasesize;

   // sanity-check
   if(str == NULL||*str==0)
      return -1;
   //printf("newline:%s", str);
   if(str[0] == ' ')
      str = skip_space(str);
#ifdef ENABLE_ERASE_CHECK
   // partition_name mount_point filesystem filename min_size mount_dev(optional)
   sscanf(str, "%s %s %s %s %Ld %d %s", partition_name, mount_point, filesystem, filename, &min_size, &bErase, mount_dev);
#else
   // partition_name mount_point filesystem filename min_size mount_dev(optional)
   sscanf(str, "%s %s %s %s %Ld %s", partition_name, mount_point, filesystem, filename, &min_size, mount_dev);
#endif

   	if (_pTargetPartDesc == NULL) {
		rtk_part = find_part_by_part_name((struct t_PARTDESC*)&rtk_part_list, partition_name);
	} else {
		rtk_part = find_part_by_part_name(_pTargetPartDesc, partition_name);
	}

   // sanity-check
   if(rtk_part == NULL)
   {
      install_debug("Can't add more partition(%s)\r\n", str);
      return -1;
   }
   if(FS_TYPE_UNKOWN == inv_efs_by_str(filesystem))
   {
      install_debug("Unkown Filesystem(%s)\r\n", str);
      return -1;
   }
   if(FS_TYPE_YAFFS2 == inv_efs_by_str(filesystem) && min_size<DEFAULT_YAFFS_SIZE)
      min_size = DEFAULT_YAFFS_SIZE;

   // show info, mount_dev should start with "/dev"
	if (!strncmp(mount_dev, "/dev", 4)) {
#ifdef ENABLE_ERASE_CHECK
   		install_info("Add partition(%s), mount_dev(%s), mount_point(%s), filesystem(%s:%d), size(%lld), bErase(%d)\r\n",
   		partition_name, mount_point, mount_dev, filesystem, inv_efs_by_str(filesystem), min_size, bErase);
#else
   		install_info("Add partition(%s), mount_dev(%s), mount_point(%s), filesystem(%s:%d), size(%lld)\r\n",
   		partition_name, mount_point, mount_dev, filesystem, inv_efs_by_str(filesystem), min_size);

#endif
	} else {
#ifdef ENABLE_ERASE_CHECK
   		install_info("Add partition(%s), mount_point(%s), filesystem(%s:%d), size(%lld) bErase(%d)\r\n",
   		partition_name, mount_point, filesystem, inv_efs_by_str(filesystem), min_size, bErase);
#else
   		install_info("Add partition(%s), mount_point(%s), filesystem(%s:%d), size(%lld) \r\n",
   		partition_name, mount_point, filesystem, inv_efs_by_str(filesystem), min_size);
#endif
        }
   // fill
   sprintf(rtk_part->partition_name, "%s", partition_name);
   sprintf(rtk_part->mount_point, "%s", mount_point);
	if (!strncmp(mount_dev, "/dev", 4))
		sprintf(rtk_part->mount_dev, "%s", mount_dev);
   rtk_part->efs = inv_efs_by_str(filesystem);
   if(strcmp(filename, "none"))
      sprintf(rtk_part->filename, "%s", filename);
    rtk_part->min_size = min_size;
#ifdef ENABLE_ERASE_CHECK
    rtk_part->bErase = bErase;
#endif

   rtk_part_list_sort[id] = rtk_part;
   id++;

   return 0;
}

int partial(char* str, struct t_rtkimgdesc* prtkimg, eMode mode)
{
	enum FWTYPE efwtype;
	char keyword[12];
	if ( str == NULL||*str==0 )
		return -1;

	if(str[0] == ' ')
		str = skip_space(str);

	install_log("partial(%s)\r\n", str);

	while(*str) {
		sscanf(str, "%s", keyword);
		install_ui("[%s]", keyword);
		for ( efwtype=FWTYPE(FW_KERNEL);efwtype<FW_BOOTCODE;efwtype=FWTYPE(efwtype+1) ) {
			if(strcmp(inv_by_fwtype(efwtype), keyword) == 0) {
				prtkimg->fw[efwtype].pass[mode] = PARTIAL_PASS;
            		break;
         		}
		}
		if(efwtype == FW_BOOTCODE) {
			install_debug("Can't find %s partition or firmware\r\n", keyword);
		}
		str = str + strlen(keyword);
		if(str[0] == ' ')
			str = skip_space(str);
	}
	return 0;
}

int add_install_factory_command(char* str, struct t_rtkimgdesc* prtkimg)
{
   char *str_tmp = NULL;
   int str_len = 0;

   if (0 == strncmp("ifcmd0", str, strlen("ifcmd0"))) {
      skip_quotation(skip_char(str, '='), &str_tmp, &str_len);
      strncpy(prtkimg->ifcmd0, str_tmp, str_len);
      prtkimg->ifcmd0[str_len] = '\0';
      install_log("Add ifcmd0(%d): \"%s\"\r\n", strlen(prtkimg->ifcmd0), prtkimg->ifcmd0);
   }
   else if (0 == strncmp("ifcmd1", str, strlen("ifcmd1"))) {
      skip_quotation(skip_char(str, '='), &str_tmp, &str_len);
      strncpy(prtkimg->ifcmd1, str_tmp, str_len);
      prtkimg->ifcmd1[str_len] = '\0';
      install_log("Add ifcmd1(%d): \"%s\"\r\n", strlen(prtkimg->ifcmd1), prtkimg->ifcmd1);
   }

   return 0;
}


// add code here
//
create_yn_fun(bootcode)
create_yn_fun(install_factory)
create_yn_fun(install_dtb)
create_yn_fun(update_etc)
create_yn_fun(start_customer)
create_yn_fun(ap_customer)
create_yn_fun(verify)
create_yn_fun(ignore_native_rescue)
create_yn_fun(stop_reboot)
create_yn_fun(kill_000)

create_yn_fun(only_factory)
create_yn_fun(only_bootcode)
create_yn_fun(erase_free_space)


create_yn_fun(partition_inverse)

create_yn_fun(jffs2_nocleanmarker)

create_yn_fun(safe_upgrade)
create_yn_fun(dump_flash)
create_yn_fun(backup)

#ifdef NAS_ENABLE
create_yn_fun(nas_rescue)
#endif

//
create_var_fun(install_avfile_count)
create_var_fun(install_avfile_video_size)
create_var_fun(install_avfile_audio_size)
create_var_fun(update_cavfile)

create_var_fun(reboot_delay)
create_var_fun(customer_delay)
create_var_fun(rba_percentage)
create_var_fun(logger_level)
create_var_fun(secure_boot)
create_var_fun(efuse_key)


/*Put default value here*/
void rtk_pre_config_init(struct t_rtkimgdesc* prtkimg)
{
	int i;

	prtkimg->rba_percentage = -1;
	prtkimg->install_avfile_audio_size = -1;
	prtkimg->install_avfile_video_size = -1;

	prtkimg->update_cavfile = 0xffffffff;

	// init dynamic part_info table default value

	for (i=0; i<NUM_SUPPORT_DYNAMIC_PART_TBL; i++) {
		dynamic_part_tbl[i].flash_sizeM = 0;
		memcpy(&(dynamic_part_tbl[i].partDescTbl), &rtk_part_list, sizeof(struct t_PARTDESC)*NUM_RTKPART);
	}
   	return;
}

static void rtk_post_config_init(struct t_rtkimgdesc* prtkimg)
{
	if (prtkimg->safe_upgrade) {
		install_debug("safe_upgrade is set -> ap_customer=1\r\n");
		prtkimg->ap_customer = 1;
	}

	if (prtkimg->dump_flash) {
		install_debug("dump_flash is set -> start_customer=0\r\n");
		prtkimg->start_customer = 0;
	}

	if (prtkimg->rba_percentage == -1) {
		prtkimg->rba_percentage = NAND_DEFAULT_RBA_PERCENTAGE;
		install_debug("set RBA percentage to default %d\r\n", NAND_DEFAULT_RBA_PERCENTAGE);
	}

	if (prtkimg->install_avfile_audio_size == -1) {
		prtkimg->install_avfile_audio_size = CUSTOMER_AUDIO_LOGO_DEFAULT_SIZE_KB;
		install_debug("set install avfile audio size to default %dKB\r\n", CUSTOMER_AUDIO_LOGO_DEFAULT_SIZE_KB);
	}

	if (prtkimg->install_avfile_video_size == -1) {
		prtkimg->install_avfile_video_size = CUSTOMER_VIDEO_LOGO_DEFAULT_SIZE_KB;
		install_debug("set install avfile video size to default %dKB\r\n", CUSTOMER_VIDEO_LOGO_DEFAULT_SIZE_KB);
	}

	if (prtkimg->update_cavfile == 0xffffffff) {
		prtkimg->update_cavfile = 1;
		install_debug("set update_cavfile to default %d\r\n", prtkimg->update_cavfile);
	}

	if (prtkimg->logger_level != 0) {
		prtkimg->logger_level = prtkimg->logger_level | INSTALL_INFO_LEVEL | INSTALL_UI_LEVEL  | INSTALL_FAIL_LEVEL;
		install_debug("set logger_level from 0x%x to 0x%x\r\n", gDebugPrintfLogLevel,prtkimg->logger_level);
		gDebugPrintfLogLevel = prtkimg->logger_level;
	}
}

int rtk_load_config(struct t_rtkimgdesc* prtkimg)
{
	FILE* file = NULL;
	int ret;
	char newline[512] = {0}, path[128] = {0};
	struct t_PARTDESC* pPartDesc = NULL;
    struct stat stat_buf;

	install_info("\r\n[Load configuration]\r\n");

	if(prtkimg->config.img_size == 0) {
		install_fail("Can't find config.txt\r\n");
		return -1;
	}

    if (stat(PKG_TEMP, &stat_buf) != 0) {
        snprintf(path, sizeof(path), "mkdir %s; chmod 775 %s", PKG_TEMP, PKG_TEMP);
        rtk_command(path, __LINE__, __FILE__, 0);
        memset(path, 0, sizeof(path));
    }
	snprintf(path, sizeof(path), "%s/config.txt", PKG_TEMP);
	ret = rtk_extract_file(prtkimg, &prtkimg->config, path);
	if(ret < 0) {
		install_fail("Can't extract config.txt\r\n");
		return -1;
   	}

	file = fopen(path,"r");
	if(file == NULL) {
		install_fail("Can't open config.txt\r\n");
		return -1;
	}

	install_ui("\r\n");

	rtk_pre_config_init(prtkimg);

	while(NULL != fgets(newline, sizeof(newline), file)) {
		if((newline[0] == ';') || (newline[0] == '#')) continue;

		del_cr_lf(newline);
		// add code here
		create_yn_match(bootcode)
		create_yn_match(install_factory)
		create_yn_match(install_dtb)
		create_yn_match(verify)
		create_yn_match(ignore_native_rescue)
		create_yn_match(stop_reboot)
		create_yn_match(update_etc)
		create_yn_match(start_customer)
		create_yn_match(kill_000)
		create_yn_match(partition_inverse)
		create_yn_match(only_factory)
		create_yn_match(only_bootcode)
		create_yn_match(jffs2_nocleanmarker)
		create_yn_match(ap_customer)
		create_yn_match(dump_flash)

		create_yn_match(erase_free_space)
		create_yn_match(safe_upgrade)
		create_yn_match(backup)

#ifdef NAS_ENABLE
		create_yn_match(nas_rescue)
#endif

		create_var_match(install_avfile_count)
		create_var_match(install_avfile_video_size)
		create_var_match(install_avfile_audio_size)
		create_var_match(update_cavfile)

		create_var_match(reboot_delay)
		create_var_match(customer_delay)
		create_var_match(rba_percentage)
		create_var_match(logger_level)

		create_var_match(secure_boot)
		create_var_match(efuse_key)

		if ( 0 == strncmp("part_targetM", newline, 11) ) {
			pPartDesc = add_dynamic_tblInfo(skip_char(newline+11, '='));
			continue;
		}

		if ( 0 == strncmp("part", newline, 4) ) {
			add_partition(skip_char(newline+4, '='), prtkimg, pPartDesc);
			continue;
		}

		if ( 0 == strncmp("fw", newline, 2) ) {
			add_firmware(skip_char(newline+2, '='), prtkimg);
			continue;
		}

		if ( 0 == strncmp("up_pass", newline, 7) ) {
			partial(skip_char(newline+7, '='), prtkimg, _UPGRAD);
			continue;
		}

		if ( 0 == strncmp("ins_pass", newline, 8) ) {
			partial(skip_char(newline+8, '='), prtkimg, _INSTALL);
			continue;
		}

		if ( 0 == strncmp("ifcmd", newline, strlen("ifcmd") )) {
			add_install_factory_command(newline, prtkimg);
			continue;
		}

		if ( 0 == strncmp("version", newline, strlen("version") )) {
			char *tmp = strchr(newline, '"');
			if( tmp ) {
				strncpy( prtkimg->version, tmp+1, strlen(tmp+1)-1 );
				continue;
			}
		}

		fflush(stdout);
	}
	fclose(file);
	rtk_post_config_init(prtkimg);
	return 0;
}

