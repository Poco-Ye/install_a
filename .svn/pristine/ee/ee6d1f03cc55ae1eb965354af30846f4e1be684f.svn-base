#ifndef RTK_PROGRAMMER
#define RTK_PROGRAMMER

typedef enum e_pm_type {
   ePROGRAMMER_NOT_DEFINED,
   ePROGRAMMER_XELTEK_5400GP,
   ePROGRAMMER_LEAP_SU6000,
   ePROGRAMMER_HILO_FLASH100,   
   ePROGRAMMER_UNKNOWN
} e_pm_type_t; 

typedef enum e_write_def_mode {
   eWRITE_PROGRAMMER_DEF_NOT_DEFINED,
   eWRITE_PROGRAMMER_HEAD,
   eWRITE_PROGRAMMER_HWSETTING,
   eWRITE_PROGRAMMER_HASHTARGET,
   eWRITE_PROGRAMMER_RESCUE_N_LOGO,
   eWRITE_PROGRAMMER_BOOTCODE_PACKED,
   eWRITE_PROGRAMMER_AP,
   eWRITE_PROGRAMMER_TAIL,  
} e_write_def_mode_t; 

const e_pm_type_t find_programmer_model_by_pmtype(char *pname);
const char *find_pmtype_by_programmer_model(enum e_pm_type pmtype);
int write_programmer_init(struct t_rtkimgdesc* prtkimgdesc, unsigned int *reserved_hwsetting_end_block_size, unsigned int *reserved_hashtarget_block_size, unsigned int *reserved_rescue_block_size);
int write_programmer_def_file_wrapper(const struct t_rtkimgdesc* prtkimgdesc, const unsigned int mode, const unsigned int start_block=0, const unsigned int process_block_size=0, const unsigned int reserved_block_size=0);
int final_programmer_process(const struct t_rtkimgdesc* prtkimgdesc);
int print_programmer_output_info(const struct t_rtkimgdesc* prtkimgdesc);

#endif /* End of RTK_PROGRAMMER */
