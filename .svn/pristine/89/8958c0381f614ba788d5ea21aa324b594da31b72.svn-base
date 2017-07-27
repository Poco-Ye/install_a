#ifndef RTK_CUSTOMER
#define RTK_CUSTOMER



int rtk_customer_init(struct t_rtkimgdesc* prtkimgdesc, int noExtract=0);
int rtk_customer_write_progress(struct t_rtkimgdesc* prtkimgdesc, E_TAG tag);
int rtk_customer_write_increase_progressbar(struct t_rtkimgdesc* prtkimgdesc, const int percentage);
int rtk_customer_write_burn_result(const struct t_rtkimgdesc* prtkimgdesc, const int err_code);
int rtk_customer_write_burn_partname(const struct t_rtkimgdesc* prtkimgdesc, const enum E_TAG tag);
int rtk_customer_write_burn_partname(FILE* customer_fp, const char* part_msg);
int rtk_customer_write_burn_dhc(const struct t_rtkimgdesc* prtkimgdesc);


#endif /* End of RTK_CUSTOMER */
