/********************************************************************
*
* @file rtk_customer.cpp
*
*   customer API.
*
*
* @log
*                  Andy Teng                       Created 2012/06/03
*
********************************************************************/

////////////////////////STANDARD INCLUDE FILES///////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

///////////////////////PROJECT INCLUDE FILES/////////////////////////
#include <rtk_common.h>
#include <rtk_burn.h>   // burn bootcode & patition
#include <rtk_boottable.h>
#include <rtk_customer.h>
#include <rtk_def.h>
#include <rtk_config.h>

/////////////////////////LOCAL DEFINITIONS///////////////////////////
#define ATUO_REBOOT_STRING                             "AUTO_REBOOT\n"
#define STOP_REBOOT_STRING                             "STOP_REBOOT\n"
#define START_BURN_STRING                              "START_BURN\n"
#define PROGRESS_START_STRING                          "BURN_PROGRESS:"
#define PROGRESS_MESSAGE                               PROGRESS_START_STRING"%d\n"
#define BURN_PARTNAME_START_STRING                     "PARTNAME:"
#define BURN_PARTNAME_MESSAGE                          BURN_PARTNAME_START_STRING"%s\n"
#define BURN_SUCCESS_STRING                            "BURN_SUCCESS\n"
#define BURN_FAIL_STRING                               "BURN_FAIL\n"
#define BURN_FAIL_ERROR_MESSAGE_START_STRING           "ERR_MSG:"
#define BURN_FAIL_ERROR_MESSAGE                        BURN_FAIL_ERROR_MESSAGE_START_STRING"#%d %s\n"
#define BURN_FAIL_ERROR_MESSAGE_DETAIL_START_STRING    "ERR_MSG_DETAIL:"
#define BURN_FAIL_ERROR_MESSAGE_DETAIL                 BURN_FAIL_ERROR_MESSAGE_DETAIL_START_STRING"#%d %s\n"
#define VERSION_START_STRING                           "VERSION:"
#define VERSION_MESSAGE                                VERSION_START_STRING"%s\n"
#define BURN_DHC_FW                                    "BURN_DHC_FW"


#define IPC_FIFO_FILE_INSTALLER_W                      "/tmp/upgfifo"


//////////////////////////STATIC FUNCTIONS///////////////////////////
static int rtk_customer_write_string(FILE* customer_fp, const char* msg);

//////////////////////////STATIC VARIABLES///////////////////////////
static unsigned long long rtk_burned_size;

/////////////////////////////FUNCTIONS///////////////////////////////
static inline int rtk_customer_write_string(FILE* customer_fp, const char* msg)
{
   //sanity-check
   if (customer_fp == NULL) {
      install_debug("error! customer write error\r\n");
      return -_eRTK_GENERAL_FAIL;
   }
   //install_debug("write to customer:\"%s\" strlen(%d)\r\n", msg, strlen(msg));

   fputs(msg, customer_fp);
   fflush(customer_fp);
   fflush(stdout);
   return _eRTK_SUCCESS;
}

int rtk_customer_init(struct t_rtkimgdesc* prtkimgdesc, int noExtract)
{
	int i;
	char cmd[128] = {0};
	char* customer_path = NULL;
	if (prtkimgdesc->ap_customer == 0) {
		if (noExtract) {
          	// temporarily I define the customer /font.ttf should be the same path with install target
            // Maybe changed in the future.
			char curPath[128] = {'\0'};
        	int idx = 0;
        	char* ptr = strrchr(prtkimgdesc->tarinfo.tarfile_path, '/');
        	if (ptr != NULL) {
				strncpy(curPath, prtkimgdesc->tarinfo.tarfile_path, (int)(ptr-prtkimgdesc->tarinfo.tarfile_path));
            	printf("curPath:%s\n", curPath);
            	idx = strlen(curPath);
            	strcat(curPath,"/"FONT_FILENAME);
            	if (access(curPath, F_OK ) != -1) {  // file exists
                	sprintf(cmd, "cp %s /tmp",curPath);
                	system(cmd);
            	} else {

            	}
            	strcpy(curPath+idx, "/"CUSTOMER_BINARY);
            	if (access(curPath, F_OK ) != -1) {  // file exists
                	prtkimgdesc->start_customer = 1;
                	snprintf(cmd, sizeof(cmd), "cp %s  /tmp", curPath);
                	system(cmd);
            	}  else {
                	install_debug("Can't find customer!\r\n");
                	prtkimgdesc->customer_fp = NULL;
                	prtkimgdesc->start_customer = 0;
                	return -_eRTK_GENERAL_FAIL;

            	}
				snprintf(cmd, sizeof(cmd), "chmod 777 %s", CUSTOMER_PATH);
				rtk_command(cmd, __LINE__, __FILE__, 0);

            	customer_path = strdup(CUSTOMER_PATH);
        	}
    	} else {
            // ALSADaemon will use /dev/graphics/fb0,
            // SDL lib will use /dev/fb0,
            // check where the fb0 is, then make symbolic link.
            #if 1
            if( ! access("/dev/graphics/fb0", F_OK) ) {
                rtk_command("ln -s /dev/graphics/fb0 /dev/fb0", __LINE__, __FILE__, 0);
            }
            else if( ! access("/dev/fb0", F_OK) ) {
                rtk_command("mkdir /dev/graphics/", __LINE__, __FILE__, 0);
                rtk_command("ln -s /dev/fb0 /dev/graphics/fb0", __LINE__, __FILE__, 0);
            }
            else {
                install_log("no fb0 found, stop IPC with ap\n");
                prtkimgdesc->customer_fp = NULL;
                prtkimgdesc->start_customer = 0;
                return -_eRTK_GENERAL_FAIL;
            }

            if (rtk_extract_file(prtkimgdesc, &prtkimgdesc->alsadaemon, ALSADAEMON_BIN) != 0) {
                install_debug("Can't extract ALSADaemon!\n");
                prtkimgdesc->customer_fp = NULL;
                prtkimgdesc->start_customer = 0;
                return -_eRTK_GENERAL_FAIL;
            }
            install_log("Start ALSADaemon(%s)\n", ALSADAEMON_BIN);
            rtk_command("chmod 777 "ALSADAEMON_BIN, __LINE__, __FILE__, 0);
            sprintf(cmd, " %s &",ALSADAEMON_BIN);
            rtk_command(cmd, __LINE__, __FILE__, 0);
            // wait some time for ALSADaemon to start..
            sleep(3);
			#endif
            if (rtk_extract_file(prtkimgdesc, &prtkimgdesc->customer, CUSTOMER_PATH) != 0) {
                install_debug("Can't extract customer!\r\n");
                prtkimgdesc->customer_fp = NULL;
                prtkimgdesc->start_customer = 0;
                return -_eRTK_GENERAL_FAIL;
            }
            sprintf(cmd, "mkdir -p /tmp/installUI;tar xvf %s -C /tmp/installUI", CUSTOMER_PATH);
            rtk_command(cmd, __LINE__, __FILE__, 0);

            customer_path = strdup("/tmp/installUI/install_UI.exe");
        }

		install_log("[Start customer]\r\n\r\n");
		snprintf(cmd, sizeof(cmd), "chmod 777 %s", customer_path);
		rtk_command(cmd, __LINE__, __FILE__, 0);

		snprintf(cmd, sizeof(cmd), "%s %s", customer_path, prtkimgdesc->tarinfo.tarfile_path);
		//install_test("%s", cmd);

		prtkimgdesc->customer_fp = popen(cmd, "w");
		if (customer_path)
			free(customer_path);
		for (i = prtkimgdesc->customer_delay; i > 0; i--) {
			install_log("wait for %d seconds...\r", i);
			fflush(stdout);
			sleep(1);
		}
		install_log("                                         \r\n");

		sprintf(cmd, VERSION_MESSAGE, prtkimgdesc->version);
		rtk_customer_write_string(prtkimgdesc->customer_fp, cmd);
		/*
		if(prtkimgdesc->stop_reboot == 1) {
			rtk_customer_write_string(prtkimgdesc->customer_fp, STOP_REBOOT_STRING);
		} else {
			rtk_customer_write_string(prtkimgdesc->customer_fp, ATUO_REBOOT_STRING);
		}

		rtk_customer_write_string(prtkimgdesc->customer_fp, START_BURN_STRING);
		*/

		prtkimgdesc->progress = 0;
	}
	else
	{
      	/* special case for ssu upgrade */

		if (access(IPC_FIFO_FILE_INSTALLER_W, F_OK)) {
         		install_log("\"%s\" not found, stop IPC with ap.\r\n", IPC_FIFO_FILE_INSTALLER_W);
         		prtkimgdesc->customer_fp = NULL;
         		prtkimgdesc->start_customer = 0;
         		return -_eRTK_GENERAL_FAIL;
      	}

      	if ((prtkimgdesc->customer_fp = fopen(IPC_FIFO_FILE_INSTALLER_W, "w")) == NULL) {
         		prtkimgdesc->start_customer = 0;
         		install_log("Can open fifo file\r\n");
         		return -_eRTK_GENERAL_FAIL;
      	}

      	if(prtkimgdesc->stop_reboot == 1) {
         		rtk_customer_write_string(prtkimgdesc->customer_fp, STOP_REBOOT_STRING);
      	} else {
         		rtk_customer_write_string(prtkimgdesc->customer_fp, ATUO_REBOOT_STRING);
      	}

      	rtk_customer_write_string(prtkimgdesc->customer_fp, START_BURN_STRING);

		prtkimgdesc->progress = 0;
   	}

	rtk_burned_size = 0;
	return _eRTK_SUCCESS;
}

int rtk_customer_write_progress(struct t_rtkimgdesc* prtkimgdesc, E_TAG tag)
{
	char msg[128];
	FWTYPE fwType = etag_to_fwtype(tag);

	if( fwType == FW_UNKNOWN ) {
		return _eUNKOWN_ERROR_CODE;
	}

	if( prtkimgdesc->fw[ fwType ].flash_allo_size ) {
#ifdef NAS_ENABLE
		if( fwType >= FW_ROOTFS && fwType <= FW_USR_LOCAL_ETC )
			rtk_burned_size += prtkimgdesc->fw[ fwType ].img_size;
		else
#endif
		rtk_burned_size += prtkimgdesc->fw[ fwType ].flash_allo_size;
		sprintf(msg, PROGRESS_MESSAGE, (int)((rtk_burned_size*100)/prtkimgdesc->total_alloc_size));
		rtk_customer_write_string(prtkimgdesc->customer_fp, msg);
	}
   	return _eRTK_SUCCESS;
}

int rtk_customer_write_increase_progressbar(struct t_rtkimgdesc* prtkimgdesc, const int percentage)
{
#if (0)	// UI will take care the progress bar update
   char msg[128] = {0};

   prtkimgdesc->progress += percentage;
   if (prtkimgdesc->progress > 100)
      prtkimgdesc->progress = 100;
   snprintf(msg, sizeof(msg), PROGRESS_MESSAGE, prtkimgdesc->progress);
   rtk_customer_write_string(prtkimgdesc, msg);
#endif
   return _eRTK_SUCCESS;
}

int rtk_customer_write_burn_result(const struct t_rtkimgdesc* prtkimgdesc, const int err_code)
{
   char msg[256] = {0};
   int ercode = 0;

   //sanity-check
   if (err_code > _eUNKOWN_ERROR_CODE || err_code < 0)
      ercode = _eUNKOWN_ERROR_CODE;
   else
      ercode = err_code;

   if (ercode == 0) {
      rtk_customer_write_string(prtkimgdesc->customer_fp, BURN_SUCCESS_STRING);
   }
   else {
      rtk_customer_write_string(prtkimgdesc->customer_fp, BURN_FAIL_STRING);
      snprintf(msg, sizeof(msg), BURN_FAIL_ERROR_MESSAGE, ercode, _error_info[ercode]._error_msg_short);
      rtk_customer_write_string(prtkimgdesc->customer_fp, msg);
      snprintf(msg, sizeof(msg), BURN_FAIL_ERROR_MESSAGE_DETAIL, ercode, _error_info[ercode]._error_msg_detail);
      rtk_customer_write_string(prtkimgdesc->customer_fp, msg);
      //install_fail("\r\n   ERR_CODE: #%d\r\n    ERR_MSG: %s\r\n ERR_DETAIL: %s\r\n", ercode, _error_info[ercode]._error_msg_short, _error_info[ercode]._error_msg_detail);
   }

   return _eRTK_SUCCESS;
}

int rtk_customer_write_burn_partname(const struct t_rtkimgdesc* prtkimgdesc, const enum E_TAG tag)
{
   char msg[128] = {0};
   int etag = 0;

   //sanity-check
   if (tag > TAG_COMPLETE || tag < TAG_UNKNOWN)
      etag = TAG_UNKNOWN;
   else
      etag = tag;

   sprintf(msg, BURN_PARTNAME_MESSAGE, TAG_PART_NAME[etag]);
   rtk_customer_write_string(prtkimgdesc->customer_fp, msg);

   return _eRTK_SUCCESS;
}

int rtk_customer_write_burn_partname(FILE* customer_fp, const char* part_msg)
{
   char msg[128] = {'\0'};

    if (part_msg == NULL)
        return _eRTK_GENERAL_FAIL;

   sprintf(msg, BURN_PARTNAME_MESSAGE, part_msg);
   return rtk_customer_write_string(customer_fp, msg);

}

int rtk_customer_write_burn_dhc(const struct t_rtkimgdesc* prtkimgdesc)
{
   rtk_customer_write_string(prtkimgdesc->customer_fp, BURN_DHC_FW);

   return _eRTK_SUCCESS;
}

