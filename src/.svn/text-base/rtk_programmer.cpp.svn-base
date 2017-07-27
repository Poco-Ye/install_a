/********************************************************************
*
* @file rtk_programmer.cpp
*
*   programmer API for offline generate bin. 
*   
* 
* @log
*                  Andy Teng                       Created 2012/09/03
*
********************************************************************/

////////////////////////STANDARD INCLUDE FILES///////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

///////////////////////PROJECT INCLUDE FILES/////////////////////////
#include <rtk_programmer.h>
#include <rtk_common.h>
#include <rtk_def.h>
#include <rtk_imgdesc.h>


/////////////////////////LOCAL DEFINITIONS///////////////////////////
#define PROGRAMMER_XELTEK_5400GP_STRING               "xeltek_5400gp"
#define PROGRAMMER_LEAP_SU6000_STRING                 "leap_su6000"
#define PROGRAMMER_HILO_FLASH100_STRING               "hilo_flash100"

const char programmer_leap_def_file_head[] = { 0x47, 0x52, 0x4F, 0x55, 0x50, 0x20, 0x44, 0x45, 0x46, 0x49, 0x4E, 0x45, 0x32, 0x00, 0x00, 0x00}; //; GROUP DEFINE2...

/* for hilo system */
#define HWSETTING_OUTPUT_FILENAME                     "hwsetting.bin"
#define HASHTARGET_OUTPUT_FILENAME                    "bootcode.bin"
#define RESCUE_N_LOGO_OUTPUT_FILENAME                 "logo_and_data.bin"
#define AP_OUTPUT_FILENAME                            "nand.bin"

//////////////////////////STATIC FUNCTIONS///////////////////////////
static int write_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc, const unsigned int start_block, const unsigned int end_block, const unsigned int process_block_size, const char *partname=NULL);
static int write_head_of_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc);
static int write_tail_of_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc);

//////////////////////////STATIC VARIABLES///////////////////////////
unsigned int hwsetting_start_block = 0, hwsetting_block_size = 0;
unsigned int hashtarget_start_block = 0, hashtarget_block_size = 0;
unsigned int rescue_n_logo_start_block = 0, rescue_n_logo_block_size = 0;
unsigned int ap_start_block = 0, ap_block_size = 0;

/////////////////////////////FUNCTIONS///////////////////////////////
const e_pm_type_t find_programmer_model_by_pmtype(char *pname)
{
   if (!strcmp(pname, PROGRAMMER_XELTEK_5400GP_STRING))
   {
      return ePROGRAMMER_XELTEK_5400GP;
   }
   else if (!strcmp(pname, PROGRAMMER_LEAP_SU6000_STRING))
   {
      return ePROGRAMMER_LEAP_SU6000;
   }
   else if (!strcmp(pname, PROGRAMMER_HILO_FLASH100_STRING)) 
   {
      return ePROGRAMMER_HILO_FLASH100;
   }
 
   return ePROGRAMMER_UNKNOWN;
}

const char *find_pmtype_by_programmer_model(enum e_pm_type pmtype)
{
   switch(pmtype)
   {
      case ePROGRAMMER_NOT_DEFINED:
         install_fail("error! pmtype not init!");
         return NULL;
         break;
      case ePROGRAMMER_XELTEK_5400GP:
         return PROGRAMMER_XELTEK_5400GP_STRING;
         break;
      case ePROGRAMMER_LEAP_SU6000:
         return PROGRAMMER_LEAP_SU6000_STRING;
         break;
      case ePROGRAMMER_HILO_FLASH100:
         return PROGRAMMER_HILO_FLASH100_STRING;
         break;  
      default:
         install_fail("error! pmtype unknown! %d", pmtype);
         return NULL;
         break;
   }
   
   return NULL;
}

int write_programmer_init(struct t_rtkimgdesc* prtkimgdesc, unsigned int *reserved_hwsetting_end_block_size, unsigned int *reserved_hashtarget_block_size, unsigned int *reserved_rescue_block_size)
{
   prtkimgdesc->epmtype = find_programmer_model_by_pmtype(prtkimgdesc->flash_programmer_model);
   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_XELTEK_5400GP:
         *reserved_hwsetting_end_block_size = 2;
         *reserved_hashtarget_block_size = 2;
         *reserved_rescue_block_size = 2;
         break;
      case ePROGRAMMER_HILO_FLASH100:
         //fall through
      case ePROGRAMMER_LEAP_SU6000:
         *reserved_hwsetting_end_block_size = *reserved_hashtarget_block_size = *reserved_rescue_block_size = 0;
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         return -1;
         break;
   }
   
   return 0;
}

int write_programmer_def_file_wrapper(const struct t_rtkimgdesc* prtkimgdesc, const unsigned int mode, const unsigned int start_block, const unsigned int process_block_size, const unsigned int reserved_block_size)
{
   install_debug("mode=%d, start_block=0x%x(%u), process_block_size=0x%x(%u), reserved_block_size=0x%x(%u).\r\n", mode, start_block, start_block, process_block_size, process_block_size, reserved_block_size, reserved_block_size);
   
   switch (mode)
   {
      case eWRITE_PROGRAMMER_HWSETTING:
            switch(prtkimgdesc->epmtype) {
               case ePROGRAMMER_HILO_FLASH100:
                  if (hwsetting_start_block == 0 && hwsetting_block_size == 0) {
                     hwsetting_start_block = start_block;
                     hwsetting_block_size = process_block_size;
                     write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size, HWSETTING_OUTPUT_FILENAME);
                  }
                  break;
               case ePROGRAMMER_XELTEK_5400GP:
                  return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size);
                  break;
               case ePROGRAMMER_LEAP_SU6000:
                  //do noting               
                  break;
               default:
                  install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
                  return -1;
                  break;
            }
            break;
      case eWRITE_PROGRAMMER_HASHTARGET:
         switch(prtkimgdesc->epmtype) {
            case ePROGRAMMER_HILO_FLASH100:
               if (hashtarget_start_block == 0 && hashtarget_block_size == 0) {
                  hashtarget_start_block = start_block;
                  hashtarget_block_size = process_block_size;
                  write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size, HASHTARGET_OUTPUT_FILENAME);
               }
               break;
            case ePROGRAMMER_XELTEK_5400GP:
               return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size);
               break;
            case ePROGRAMMER_LEAP_SU6000:
               //do noting               
               break;
            default:
               install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
               return -1;
               break;
         }
         break;
      case eWRITE_PROGRAMMER_RESCUE_N_LOGO:
         switch(prtkimgdesc->epmtype) {
            case ePROGRAMMER_HILO_FLASH100:
               if (rescue_n_logo_start_block == 0 && rescue_n_logo_block_size == 0) {
                  rescue_n_logo_start_block = start_block;
                  rescue_n_logo_block_size = process_block_size;
                  write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size, RESCUE_N_LOGO_OUTPUT_FILENAME);
               }
               break;
            case ePROGRAMMER_XELTEK_5400GP:
               return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size);
               break;
            case ePROGRAMMER_LEAP_SU6000:
               //do noting               
               break;
            default:
               install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
               return -1;
               break;
         }
         break;
      case eWRITE_PROGRAMMER_BOOTCODE_PACKED:
         switch(prtkimgdesc->epmtype) {
            case ePROGRAMMER_HILO_FLASH100:
               //fall through
            case ePROGRAMMER_XELTEK_5400GP:
               //do noting               
               break;
            case ePROGRAMMER_LEAP_SU6000:
               return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size);               
               break;
            default:
               install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
               return -1;
               break;
         }
         break;
      case eWRITE_PROGRAMMER_AP:
         switch(prtkimgdesc->epmtype) {
            case ePROGRAMMER_HILO_FLASH100:
               if (ap_start_block == 0 && ap_block_size == 0) {
                  ap_start_block = start_block;
                  ap_block_size = process_block_size;
                  write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size, AP_OUTPUT_FILENAME);
               }
               break;
            case ePROGRAMMER_XELTEK_5400GP:
               return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size+reserved_block_size-1, process_block_size);             
               break;
            case ePROGRAMMER_LEAP_SU6000:
               return write_programmer_def_file(prtkimgdesc, start_block, start_block+process_block_size-1, process_block_size);            
               break;
            default:
               install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
               return -1;
               break;
         }
         break;
      case eWRITE_PROGRAMMER_HEAD:
         return write_head_of_programmer_def_file(prtkimgdesc);
         break;
      case eWRITE_PROGRAMMER_TAIL:
         return write_tail_of_programmer_def_file(prtkimgdesc);
         break;
      case eWRITE_PROGRAMMER_DEF_NOT_DEFINED:
      default:
         install_fail("error! undefined mode %d\r\n", prtkimgdesc->epmtype);
         return -1;
         break;
   }

   return 0;
}

static int write_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc, const unsigned int start_block, const unsigned int end_block, const unsigned int process_block_size, const char *partname)
{
   int wret, fd;
   unsigned int dummy_data;
   char data[512] = {0};

   if ((fd = open(DEFAULT_PROGRAMMER_DEF_FILE, O_RDWR|O_SYNC|O_CREAT|O_APPEND, 0644)) < 0 ) {
      install_fail("error! open %s file fail %d.", DEFAULT_PROGRAMMER_DEF_FILE, fd);
      return -1;
   }

   // sanity-check
   if (start_block > end_block) {
      install_fail("error! start block %d > end block %d", start_block, end_block);
      return -1;
   }

   printf("start block:0x%x(%u), end block:0x%x(%u), process size:0x%x(%u)\r\n", start_block, start_block, end_block, end_block, process_block_size, process_block_size);

   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_HILO_FLASH100:
         if (partname) {
            snprintf(data, sizeof(data), "%18s : start block %3u, block size %3u\r\n", partname, start_block, process_block_size);
            wret = write(fd, (void*) data, strlen(data));
         }
         break;
      case ePROGRAMMER_XELTEK_5400GP:
         dummy_data = 0xffffffff;
         wret = write(fd, (void*) &start_block, sizeof(start_block));
         wret = write(fd, (void*) &end_block, sizeof(end_block));
         wret = write(fd, (void*) &process_block_size, sizeof(process_block_size));
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         break;
      case ePROGRAMMER_LEAP_SU6000:
         dummy_data = 0x00000001;
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         wret = write(fd, (void*) &start_block, sizeof(start_block));
         wret = write(fd, (void*) &end_block, sizeof(end_block));
         wret = write(fd, (void*) &process_block_size, sizeof(process_block_size));
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         close(fd);
         return -1;         
         break;
   }

   close(fd);
   
   return 0;
}

static int write_head_of_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc)
{
   int wret, fd;
   char data[512] = {0};

   if ((fd = open(DEFAULT_PROGRAMMER_DEF_FILE, O_RDWR|O_SYNC|O_CREAT|O_APPEND, 0644)) < 0 ) {
      install_fail("error! open %s file fail %d.", DEFAULT_PROGRAMMER_DEF_FILE, fd);
      return -1;
   }
      
   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_HILO_FLASH100:
         snprintf(data, sizeof(data), "Realtek Partition Table\r\n- Start -\r\n");
         wret = write(fd, (void*) data, strlen(data));
         break;
      case ePROGRAMMER_XELTEK_5400GP:
         //do noting
         break;
      case ePROGRAMMER_LEAP_SU6000:
         wret = write(fd, (void*) &programmer_leap_def_file_head, sizeof(programmer_leap_def_file_head));
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         close(fd);
         return -1;
         break;
   }

   close(fd);

   return 0;
}

static int write_tail_of_programmer_def_file(const struct t_rtkimgdesc* prtkimgdesc)
{
   int wret, fd;
   unsigned int dummy_data;
   char data[512] = {0};

   if ((fd = open(DEFAULT_PROGRAMMER_DEF_FILE, O_RDWR|O_SYNC|O_CREAT|O_APPEND, 0644)) < 0 ) {
      install_fail("error! open %s file fail %d.", DEFAULT_PROGRAMMER_DEF_FILE, fd);
      return -1;
   }

   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_HILO_FLASH100:
         snprintf(data, sizeof(data), "- End -\r\n");
         wret = write(fd, (void*) data, strlen(data));
         break;
      case ePROGRAMMER_XELTEK_5400GP:
         //fall through
      case ePROGRAMMER_LEAP_SU6000:
         dummy_data = 0xffffffff;
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         wret = write(fd, (void*) &dummy_data, sizeof(dummy_data));
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         close(fd);
         return -1; 
         break;
   }  

   close(fd);

   return 0;
}

int final_programmer_process(const struct t_rtkimgdesc* prtkimgdesc)
{
   int ret;
   char cmd[256] = {0};
   unsigned int virt_block_size;

   //fix me
   virt_block_size = prtkimgdesc->mtd_erasesize/512*(512+16);

   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_XELTEK_5400GP:
         //fall through
      case ePROGRAMMER_LEAP_SU6000:
         snprintf(cmd, sizeof(cmd), "mv %s %s", DEFAULT_TEMP_OUTPUT, prtkimgdesc->output_path);
         if((ret = rtk_command(cmd, __LINE__, __FILE__, 0)) < 0) {
            return -1;
         } 
         snprintf(cmd, sizeof(cmd), "mv %s %s.def", DEFAULT_PROGRAMMER_DEF_FILE, prtkimgdesc->output_path);
         if((ret = rtk_command(cmd, __LINE__, __FILE__, 0)) < 0) {
            return -1;
         }    
         break;
      case ePROGRAMMER_HILO_FLASH100:
         snprintf(cmd, sizeof(cmd), "dd if=%s of=%s/%s bs=%u skip=%u count=%u", prtkimgdesc->mtd_path, DEFAULT_OUTPUT_DIR, HWSETTING_OUTPUT_FILENAME, virt_block_size, hwsetting_start_block, hwsetting_block_size);
         ret = rtk_command(cmd, __LINE__, __FILE__, 1);
         snprintf(cmd, sizeof(cmd), "dd if=%s of=%s/%s bs=%u skip=%u count=%u", prtkimgdesc->mtd_path, DEFAULT_OUTPUT_DIR, HASHTARGET_OUTPUT_FILENAME, virt_block_size, hashtarget_start_block, hashtarget_block_size);
         ret = rtk_command(cmd, __LINE__, __FILE__, 1);
         snprintf(cmd, sizeof(cmd), "dd if=%s of=%s/%s bs=%u skip=%u count=%u", prtkimgdesc->mtd_path, DEFAULT_OUTPUT_DIR, RESCUE_N_LOGO_OUTPUT_FILENAME, virt_block_size, rescue_n_logo_start_block, rescue_n_logo_block_size);
         ret = rtk_command(cmd, __LINE__, __FILE__, 1);
         snprintf(cmd, sizeof(cmd), "dd if=%s of=%s/%s bs=%u skip=%u count=%u", prtkimgdesc->mtd_path, DEFAULT_OUTPUT_DIR, AP_OUTPUT_FILENAME, virt_block_size, ap_start_block, ap_block_size);
         ret = rtk_command(cmd, __LINE__, __FILE__, 1);         
         //remove files
         snprintf(cmd, sizeof(cmd), "rm %s %s", DEFAULT_TEMP_OUTPUT, DEFAULT_PROGRAMMER_DEF_FILE);
         ret = rtk_command(cmd, __LINE__, __FILE__, 0);         
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         return -1; 
         break;
   }  

   return 0;
}

int print_programmer_output_info(const struct t_rtkimgdesc* prtkimgdesc)
{
   int ret;
   char cmd[128] = {0};

   switch(prtkimgdesc->epmtype)
   {
      case ePROGRAMMER_XELTEK_5400GP:
         //fall through
      case ePROGRAMMER_LEAP_SU6000:
         install_log(VT100_LIGHT_GREEN"%20s: %s\r\n"VT100_NONE, "PROGRAMMER NAME", prtkimgdesc->flash_programmer_name);
         install_log(VT100_LIGHT_GREEN"%20s: %s.def\r\n"VT100_NONE, "DEF FILE", prtkimgdesc->output_path);
         install_log(VT100_LIGHT_GREEN"%20s: %s\r\n"VT100_NONE, "SMT FILE", prtkimgdesc->output_path);
         install_log(VT100_LIGHT_GREEN"%20s: %s\r\n"VT100_NONE, "PARTNAME", prtkimgdesc->flash_partname);
         install_log(VT100_LIGHT_GREEN"%20s: %u MBytes\r\n"VT100_NONE, "SIZE", prtkimgdesc->flash_size/1024/1024);
         break;
      case ePROGRAMMER_HILO_FLASH100:
         install_log(VT100_LIGHT_GREEN"%20s: %s\r\n"VT100_NONE, "PROGRAMMER NAME", prtkimgdesc->flash_programmer_name);
         install_log(VT100_LIGHT_GREEN"%20s: %s/*\r\n"VT100_NONE, "SMT FILE", DEFAULT_OUTPUT_DIR);
         install_log(VT100_LIGHT_GREEN"%20s: %s\r\n"VT100_NONE, "PARTNAME", prtkimgdesc->flash_partname);
         install_log(VT100_LIGHT_GREEN"%20s: %u MBytes\r\n"VT100_NONE, "SIZE", prtkimgdesc->flash_size/1024/1024);
         break;
      default:
         install_fail("error! undefined epmtype %d\r\n", prtkimgdesc->epmtype);
         return -1; 
         break;
   }  

   return 0;
}
