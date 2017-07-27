#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include <rtk_parameter.h>
#include <rtk_mtd.h>
#include <rtk_common.h>
#include <rtk_def.h>
#include <rtk_imgdesc.h>
#include <rtk_fwdesc.h>

#if defined(__OFFLINE_GENERATE_BIN__) || defined(PC_SIMULATE)
static int get_string_from_setting_file(const char* keystring, char *string, int str_maxlen);
static int get_value_from_setting_file(const char* keystring, unsigned int *value);
static char project_config_isinit = 0;
#endif

#ifndef USING_SYSTEM_FILE
#define _128K_BYTE	128*1024U
#define _512K_BYTE	512*1024U
#define   _2M_BYTE	2*1024*1024U
#define   _4M_BYTE	4*1024*1024U
#define   _8M_BYTE	8*1024*1024U

static char sys_param[256];
int set_system_param(struct t_rtkimgdesc* prtkimg)
{
	const unsigned int ubootSize = _512K_BYTE;
	unsigned int bootCodeStart, reservedSize, factorySize, se_storageSize;
	const char *flashType;

	switch( prtkimg->flash_type ) {
	case MTD_NANDFLASH:
		flashType = "nand";
		bootCodeStart = 0;
#ifdef NAS_ENABLE
		/* For SLC, erase block < 512KB */
		/* Min factory size is 4 blocks */
		if(prtkimg->mtd_erasesize < 0x80000)
		factorySize = prtkimg->mtd_erasesize * 4;
		else
#endif
		{
#ifdef NFLASH_LAOUT
		se_storageSize = _8M_BYTE;
		factorySize = _4M_BYTE;
#else
		factorySize = _4M_BYTE; //org is 8MB
#endif
		}

		/*
		 * size of protected area is defined as follows:
		 * NF Profile + BBT: 6 blocks
		 * hwSetting: 1 block * 4
		 * uBoot: 512 KByte * 4
		 * factory size: 8MByte
		 */
		reservedSize = SIZE_ALIGN_BOUNDARY_MORE(ubootSize, prtkimg->mtd_erasesize) * 4;
#ifdef NFLASH_LAOUT
#ifdef USE_SE_STORAGE
		reservedSize += SIZE_ALIGN_BOUNDARY_MORE(se_storageSize, prtkimg->mtd_erasesize);
#endif
#endif
		reservedSize += SIZE_ALIGN_BOUNDARY_MORE(factorySize, prtkimg->mtd_erasesize);
		reservedSize += (6+1*4) * prtkimg->mtd_erasesize;
		// add extra 20% space for safety.
		reservedSize = SIZE_ALIGN_BOUNDARY_MORE(reservedSize*1.2, prtkimg->mtd_erasesize);
		break;
#ifdef EMMC_SUPPORT
	case MTD_EMMC:
		flashType = "emmc";
		bootCodeStart = 0x20000;
#ifdef USE_SE_STORAGE
#ifdef NFLASH_LAOUT
		se_storageSize = _4M_BYTE;
		factorySize = _4M_BYTE;
		reservedSize = 0xA20000; // 0x620000+4MB
#endif
#else
		factorySize = _4M_BYTE;
		reservedSize = 0x620000;
#endif
		break;
#endif
	case MTD_DATAFLASH:
		flashType = "spi";
		bootCodeStart = 0xC000;
		reservedSize = 0x100000;
		factorySize = _128K_BYTE;
#ifdef CONFIG_BOOT_FROM_SPI
		bootCodeStart = 0x0;
		reservedSize = 0x30000;
#endif

#if defined(NAS_ENABLE) && defined(CONFIG_BOOT_FROM_SPI)
	sprintf(sys_param, "norbootcode_start=0x%x norbootcode_size=0x%x "
					   "norfactory_start=0x%x norfactory_size=0x%x boot_flash=%s",
					   bootCodeStart, 0x0,
					   reservedSize-factorySize, factorySize, flashType);
#endif
                break;
	default:
		install_fail("Non-supported flash type(%d)\n",prtkimg->flash_type);
		return -1;
	}

#ifdef NAS_ENABLE
        if(prtkimg->flash_type != MTD_DATAFLASH)
#endif
#ifdef USE_SE_STORAGE
#ifdef NFLASH_LAOUT
	sprintf(sys_param, "bootcode_start=0x%x bootcode_size=0x%x se_storage_Start=0x%x "
					   "se_storageSize=0x%x factory_start=0x%x factory_size=0x%x boot_flash=%s",
					   bootCodeStart, reservedSize-factorySize-se_storageSize, 
					   reservedSize-factorySize-se_storageSize, se_storageSize,
					   reservedSize-factorySize, factorySize, flashType);
#endif
#else
	sprintf(sys_param, "bootcode_start=0x%x bootcode_size=0x%x "
					   "factory_start=0x%x factory_size=0x%x boot_flash=%s",
					   bootCodeStart, reservedSize-factorySize,
					   reservedSize-factorySize, factorySize, flashType);
#endif
	printf("[Installer_D]sys_param(%s)\n", sys_param);
	return 0;
}
#endif

#ifndef __OFFLINE_GENERATE_BIN__
int get_parameter_value(const char* keystring, unsigned int *keyval)
{
#ifdef USING_SYSTEM_FILE
	static int is_read_file = 0;
    int ret;
#endif
	static char sys_parameters[256];
	char sys_para_tmp[256] = {0};
	char *ptr, *ptr1;
	const char *sep = ": \t\n";

#ifdef USING_SYSTEM_FILE
	if(is_read_file == 0)
	{
		memset(sys_parameters, 0 , sizeof(sys_parameters));
		ret = rtk_file_to_string(SYSTEM_PARAMETERS, sys_parameters);
		if(ret < 0)
		{
			install_debug("Can not read system parameters(%s).\n", sys_parameters);
			return -1;
		}
		is_read_file = 1;
	}
#else
	strcpy(sys_parameters, sys_param);
#endif
	// find keystring
	//install_info("system_para:[%s]\r\n", sys_parameters);
	ptr = strstr(sys_parameters, keystring);
	if(!ptr)
	{
		install_info("Can't find string(%s)\r\n", keystring);
		return -1;
	}

	sprintf(sys_para_tmp, "%s", sys_parameters);
	ptr = strtok(sys_para_tmp, sep);
	while(ptr)
	{
		if(strstr(ptr, keystring))
		{
			ptr1 = strstr(ptr, "=");
			ptr1++;
			*keyval = strtol(ptr1, &ptr, 16);
			install_debug("Got system_parameter: %s 0x%x\r\n", keystring, *keyval);
			break;
		}
		ptr = strtok(NULL, sep);
	}
	return 0;
}
#else /* else of ifndef __OFFLINE_GENERATE_BIN__ */

int get_parameter_long_value(const char* keystring, unsigned long long *keyval)
{
   unsigned int size;

   if (!strcmp(keystring,"flash_size")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, &size);
      *keyval = (unsigned long long)size*1024;
   }

   install_debug("Got system_settings: %s 0x%llx\r\n", keystring, *keyval);

   return 0;
}

int get_parameter_value(const char* keystring, unsigned int *keyval)
{
   char tmp[128] = {0};

   if (!strcmp(keystring,"factory_start"))
   {
      unsigned int flash_size = 0, factory_size = 0;

      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, &flash_size);

      get_string_from_setting_file(SETTING_STRING_FACTORY_SIZE_MANUAL, tmp, 128);
      if (tmp[0] == 'y' || tmp[0] == 'Y') {
         get_value_from_setting_file(SETTING_STRING_FACTORY_SIZE, &factory_size);
      	*keyval = flash_size*1024 - 2*factory_size;
      }
      else {
      	install_fail("factory start or factory size not initialized!!!\r\n");
         return -1;
      }
   }
   else if (!strcmp(keystring,"factory_size"))
   {
      get_string_from_setting_file(SETTING_STRING_FACTORY_SIZE_MANUAL, tmp, 128);
      if (tmp[0] == 'y' || tmp[0] == 'Y') {
         get_value_from_setting_file(SETTING_STRING_FACTORY_SIZE, keyval);
      }
      else {
      	install_fail("factory size not initialized!!!\r\n");
         return -1;
      }
   }
   else if (!strcmp(keystring,"flash_erase_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_ERASE_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
   else if (!strcmp(keystring,"flash_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
   else if (!strcmp(keystring,"flash_page_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_PAGE_SIZE, keyval);
   }
   else if (!strcmp(keystring,"flash_oob_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_OOB_SIZE, keyval);
   }
	else if (!strcmp(keystring,"flash_bp1_size")) {
		get_value_from_setting_file(SETTING_STRING_FLASH_BP1_SIZE_KB, keyval);
      *keyval = *keyval*1024;
	}
	else if (!strcmp(keystring,"flash_bp2_size")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_BP2_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
	else if (!strcmp(keystring,"flash_erased_content")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_ERASED_CONTENT, keyval);
   }

   install_debug("Got system_settings: %s 0x%x\r\n", keystring, *keyval);
	return 0;
}
#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */


#ifndef __OFFLINE_GENERATE_BIN__

// 0: fail or nand flash
// 1: spi
int check_target_flash_is_spi(void) {
   int ret = 0;
   char tmp[64] = {0};

   ret = get_parameter_string("boot_flash", tmp, 128);
   if (ret < 0) {
      install_test("can't get boot type\n");
      return -1;
   }

   if (!strcmp(tmp, "spi")) {
      return 1;
   } else if (!strcmp(tmp, "nand")){
      return 0;
   } else {
      install_test("can't get boot type\n");
      return -1;
   }

}

int get_parameter_string(const char* keystring, char *string, int str_maxlen)
{
#ifdef USING_SYSTEM_FILE
	static int is_read_file = 0;
    int ret = 0;
#endif
	static char sys_parameters[2048];
	char sys_para_tmp[256] = {0};
	char *ptr, *ptr1, *ptr2;
	const char *sep = ": \t\n";
    int length = 0;

	if (string == NULL) return -1;
	if (str_maxlen == 0) return -1;

#ifdef USING_SYSTEM_FILE
	if(is_read_file == 0)
	{
		memset(sys_parameters, 0 , sizeof(sys_parameters));
		ret = rtk_file_to_string(SYSTEM_PARAMETERS, sys_parameters);
		if(ret < 0)
		{
			install_debug("Can not read system parameters(%s).\n", sys_parameters);
			return -1;
		}
		is_read_file = 1;
	}
#else
	strcpy(sys_parameters, sys_param);
#endif


	// find keystring
	//install_info("system_para:[%s]\r\n", sys_parameters);
	ptr = strstr(sys_parameters, keystring);
	if(!ptr)
	{
		install_info("Can't find string(%s)\r\n", keystring);
		return -1;
	}

	sprintf(sys_para_tmp, "%s", sys_parameters);
	ptr = strtok(sys_para_tmp, sep);
	while(ptr)
	{
		ptr = strtok(NULL, sep);
		if(!ptr)
			continue;
		if(strstr(ptr, keystring))
		{
			ptr1 = strstr(ptr, "=");
			ptr1++;
			ptr2 = strtok(NULL, sep);
			if (ptr2) {
				length = ptr2 - ptr1;
			} else {
				length = strlen(ptr1);
			}
			//printf("ptr1 = %s\n", ptr1);
			string[0] = 0;
			if (length) {
				strcpy(string, ptr1);
			}
			//printf("string = %s\n", string);
			return 0;
		}
	}
	return -1;
}
#else /* else of ifndef __OFFLINE_GENERATE_BIN__ */
int get_parameter_string(const char* keystring, char *string, int str_maxlen)
{
   if (!strcmp(keystring,"boot_flash"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_TYPE, string, str_maxlen);
   }
   else if (!strcmp(keystring,"output_path"))
   {
      get_string_from_setting_file(SETTING_STRING_IMAGE_OUTPUT, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_partname"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PARTNAME, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_programmer_model"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PROGRAMMER_MODEL, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_programmer_name"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PROGRAMMER_NAME, string, str_maxlen);
   }
   install_debug("Got system_settings: %s %s\r\n", keystring, string);

   return 0;

}
#endif /* end of ifndef __OFFLINE_GENERATE_BIN__ */

#if defined(__OFFLINE_GENERATE_BIN__) || defined(PC_SIMULATE)

#ifdef  PC_SIMULATE
int pc_get_parameter_string(const char* keystring, char *string, int str_maxlen)
{
   if (!strcmp(keystring,"boot_flash"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_TYPE, string, str_maxlen);
   }
   else if (!strcmp(keystring,"output_path"))
   {
      get_string_from_setting_file(SETTING_STRING_IMAGE_OUTPUT, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_partname"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PARTNAME, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_programmer_model"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PROGRAMMER_MODEL, string, str_maxlen);
   }
   else if (!strcmp(keystring,"flash_programmer_name"))
   {
      get_string_from_setting_file(SETTING_STRING_FLASH_PROGRAMMER_NAME, string, str_maxlen);
   }
   install_debug("Got system_settings: %s %s\r\n", keystring, string);

   return 0;

}
int pc_get_parameter_long_value(const char* keystring, unsigned long long *keyval)
{
   unsigned int size;

   if (!strcmp(keystring,"flash_size")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, &size);
      *keyval = (unsigned long long)size*1024;
   }

   install_debug("Got system_settings: %s 0x%llx\r\n", keystring, *keyval);

   return 0;
}

int pc_get_parameter_value(const char* keystring, unsigned int *keyval)
{
   char tmp[128] = {0};

   if (!strcmp(keystring,"factory_start"))
   {
      unsigned int flash_size = 0, factory_size = 0;

      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, &flash_size);

      get_string_from_setting_file(SETTING_STRING_FACTORY_SIZE_MANUAL, tmp, 128);
      if (tmp[0] == 'y' || tmp[0] == 'Y') {
         get_value_from_setting_file(SETTING_STRING_FACTORY_SIZE, &factory_size);
      	*keyval = flash_size*1024 - 2*factory_size;
      }
      else {
      	install_fail("factory start or factory size not initialized!!!\r\n");
         return -1;
      }
   }
   else if (!strcmp(keystring,"factory_size"))
   {
      get_string_from_setting_file(SETTING_STRING_FACTORY_SIZE_MANUAL, tmp, 128);
      if (tmp[0] == 'y' || tmp[0] == 'Y') {
         get_value_from_setting_file(SETTING_STRING_FACTORY_SIZE, keyval);
      }
      else {
      	install_fail("factory size not initialized!!!\r\n");
         return -1;
      }
   }
   else if (!strcmp(keystring,"flash_erase_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_ERASE_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
   else if (!strcmp(keystring,"flash_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
   else if (!strcmp(keystring,"flash_page_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_PAGE_SIZE, keyval);
   }
   else if (!strcmp(keystring,"flash_oob_size"))
   {
      get_value_from_setting_file(SETTING_STRING_FLASH_OOB_SIZE, keyval);
   }
	else if (!strcmp(keystring,"flash_bp1_size")) {
		get_value_from_setting_file(SETTING_STRING_FLASH_BP1_SIZE_KB, keyval);
      *keyval = *keyval*1024;
	}
	else if (!strcmp(keystring,"flash_bp2_size")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_BP2_SIZE_KB, keyval);
      *keyval = *keyval*1024;
   }
	else if (!strcmp(keystring,"flash_erased_content")) {
      get_value_from_setting_file(SETTING_STRING_FLASH_ERASED_CONTENT, keyval);
   }

   install_debug("Got system_settings: %s 0x%x\r\n", keystring, *keyval);
	return 0;
}
#endif /* end of #ifdef  PC_SIMULATE */

static inline char* skip_tab_space(char* str)
{
   do
   {
      if(*str==0) return NULL;
      if(*str != 0x09 && *str != 0x20) return str;
      str++;
   } while(1);
   return NULL;
}

static inline char* skip_char(char* str, char c)
{
	do
	{
		if(*str == 0) return NULL;
		if(*str == c) return str+1;
		str++;
	} while(1);
	return NULL;
}

static inline char* skip_space(char* str)
{
	do
	{
		if(*str == 0) return NULL;
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

static inline int del_space_str(char* str)
{
   char *find_last = NULL;

	do
	{
      if ((find_last = strrchr(str, ' ')) != NULL){
         *find_last = '\0';
      } else {
         return 0;
      }
	} while(1);
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


static int get_string_from_setting_file(const char* keystring, char *string, int str_maxlen)
{
   FILE *file_fd = NULL;
   char newline[512] = {0};
   char *str_tmp = NULL;
   int str_len = 0;
   extern char gsettingpath[128];

   char *find_first = NULL;
   char parse_head_tmp[128] = {0};

   if ((file_fd = fopen(gsettingpath, "rb")) == NULL) {
      install_fail("Can't open %s\r\n", gsettingpath);
      return -1;
   }

   //printf("get_string_from_setting_file: keystring=%s\r\n", keystring);

	while(NULL != fgets(newline, sizeof(newline), file_fd))
	{

		if(newline[0] == '#') continue;

      del_cr_lf(newline);

		//install_debug("[%c][%d] newline:%s", newline[0], strlen(newline), newline);

      if ((find_first = strchr(newline, '=')) == NULL) {
         continue;
      }
      else {
         strncpy(parse_head_tmp, newline, find_first - newline);
         parse_head_tmp[find_first - newline] = '\0';
         del_space_str(parse_head_tmp);

         //install_test("parse_head_tmp=%s(%d)\r\n",parse_head_tmp, strlen(parse_head_tmp));
      }

		if(!strcmp(parse_head_tmp, keystring)) {
         //install_test("get_string_from_setting_file: find match %s\r\n", keystring);
         skip_quotation(skip_space(skip_char(newline+strlen(keystring), '=')), &str_tmp, &str_len);
         if (str_len >= str_maxlen-1) str_len = str_maxlen-1;
			strncpy(string, str_tmp, str_len);
         string[str_len] = '\0';
         //install_test("get_string_from_setting_file: string=%s string_len=%d\r\n", string, str_len);
         return 0;
      }

		fflush(stdout);
	}
	fclose(file_fd);

   if (str_tmp == NULL) return -1;

   return 0;

}

static int get_value_from_setting_file(const char* keystring, unsigned int *value)
{
   char str_tmp[128] = {0};
   char *str_end = NULL;

   if (get_string_from_setting_file(keystring, str_tmp, sizeof(str_tmp)) == 0){
      if (strncmp(str_tmp, "0x", strlen("0x")) == 0 || strncmp(str_tmp, "0X", strlen("0X")) == 0)
         *value = strtol(str_tmp, &str_end, 16);
      else
         *value = strtol(str_tmp, &str_end, 10);

      //install_test("get_value_from_setting_file: string=%s value=%u\r\n", str_tmp, *value);
   }
   else {
      //install_test("get_value_from_setting_file: cannot find %s", keystring);
      return -1;
   }

   return 0;
}


int rtk_init_project_config(struct t_rtkimgdesc* prtkimg, char *filepath)
{
   char cmd[128] = {0}, path[128] = {0};
   int ret = 0;

   snprintf(path, sizeof(path), "%s/%s", BOOTCODE_TEMP, PROJECT_CONFIG_FILE);

   if (access(path, F_OK)) {
      snprintf(path, sizeof(path), "%s.tar", BOOTCODE_TEMP);
      if(rtk_extract_file(prtkimg, &prtkimg->bootloader_tar, path) < 0)
      {
         install_fail("Can't extract bootcode\r\n");
         return -_eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT;
      }

      snprintf(cmd, sizeof(cmd), "rm -rf %s;mkdir -p %s;tar -xf %s.tar -C %s/", BOOTCODE_TEMP, BOOTCODE_TEMP, BOOTCODE_TEMP, BOOTCODE_TEMP);

      if((ret = rtk_command(cmd , __LINE__, __FILE__, 0)) < 0)
      {
         install_fail("untar %s.tar fail\r\n", BOOTCODE_TEMP);
         return -_eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT;
      }

      snprintf(path, sizeof(path), "%s/%s", BOOTCODE_TEMP, PROJECT_CONFIG_FILE);
   }

   snprintf(cmd, sizeof(cmd), "cp %s %s/%s", path, PKG_TEMP, PROJECT_CONFIG_FILE);

   if((ret = rtk_command(cmd , __LINE__, __FILE__, 0)) < 0)
   {
      install_fail("copy %s fail\r\n", PROJECT_CONFIG_FILE);
      return -_eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT;
   }

   sprintf(filepath, "%s", path);

   project_config_isinit = 1;

   install_debug("%s: %s",__FUNCTION__, filepath);

   return 0;
}


int get_value_from_project_config(struct t_rtkimgdesc* prtkimg, const char* keystring, unsigned int *value)
{
   FILE* file = NULL;
   int found = 0;
   char newline[512] = {0};
   char *str_pos = NULL, *str_end = NULL;
   static char filepath[128] = {0};

   if (project_config_isinit == 0)
   {
      rtk_init_project_config(prtkimg, filepath);
   }

   file = fopen(filepath,"r");
   if (file == NULL)
   {
      install_fail("Can't open %s\r\n", filepath);
      return -1;
   }

   while(NULL != fgets(newline, sizeof(newline), file))
   {
      if((newline[0] == '/') || (newline[0] == '*')) continue;

      del_cr_lf(newline);

      //install_test("[%c][%d] newline:%s\r\n", newline[0], strlen(newline), newline);

      //find "#define" string position
      str_pos = strstr(newline, "#define");
      if(str_pos)
      {
         //find keystring position
         str_pos = strstr(str_pos, keystring);
         if(str_pos)
         {
            str_pos = skip_tab_space(str_pos+strlen(keystring));
            if (str_pos)
            {
               if((0 == strncmp(str_pos, "0x", 2)) || (0 == strncmp(str_pos, "0X", 2)))
               {
                  *value = strtol(str_pos, &str_end, 16);
                  install_debug("%s: %s = %#x\r\n", __FUNCTION__, keystring, *value);
               }
               else
               {
                  *value = strtol(str_pos, &str_end, 10);
                  install_debug("%s: %s = %d\r\n", __FUNCTION__, keystring, *value);
               }
               found = 1;
               break;
            }
            else
            {
               fclose(file);
               return -1;
            }
         }
      }
      fflush(stdout);
   }
   fclose(file);

   if (found)
      return 0;
   else
      return -1;
}

int get_factory_size_from_bootcode(struct t_rtkimgdesc* prtkimg)
{
   char cmd[128] = {0}, path[128] = {0};
   int ret = 0;
   //unsigned int factory_size = 0;

   snprintf(path, sizeof(path), "%s.tar", BOOTCODE_TEMP);
   if(rtk_extract_file(prtkimg, &prtkimg->bootloader_tar, path) < 0)
   {
      install_fail("Can't extract bootcode\r\n");
      return -_eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT;
   }

   snprintf(cmd, sizeof(cmd), "rm -rf %s;mkdir -p %s;tar -xf %s.tar -C %s/", BOOTCODE_TEMP, BOOTCODE_TEMP, BOOTCODE_TEMP, BOOTCODE_TEMP);

   if((ret = rtk_command(cmd , __LINE__, __FILE__, 0)) < 0)
   {
      install_fail("untar %s.tar fail\r\n", BOOTCODE_TEMP);
      return -_eFILL_RTK_IMGDESC_LAYOUT_FAIL_BOOTCODE_EXTRACT;
   }

   snprintf(path, sizeof(path), "%s/%s", BOOTCODE_TEMP, RESETROM_FILENAME);
   rtk_file_to_ptr(path, 0x1c, (void *)&prtkimg->factory_size, 4);

   install_debug("get_factory_size_from_bootcode: %x\r\n", prtkimg->factory_size);

   return _eRTK_SUCCESS;

}

#endif
