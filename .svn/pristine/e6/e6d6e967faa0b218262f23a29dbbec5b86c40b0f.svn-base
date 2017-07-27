#ifndef RTK_XML_H_
#define RTK_XML_H_
enum firm_ID
{
	KERNEL_ID=0,
	AUDIO_ID,
	VIDEO_ID
	
};

const char* get_linuxKernel_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_tee_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_audioKernel_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_videoKernel_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_etc_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_rootfs_filename(struct t_rtkimgdesc* prtkimgdesc);
const char* get_resource_filename(struct t_rtkimgdesc* prtkimgdesc);
long long int get_cache_partition_size(struct t_rtkimgdesc* prtkimgdesc);
long long int get_etc_partition_size(struct t_rtkimgdesc* prtkimgdesc);
long long int get_preserve_partition_size(struct t_rtkimgdesc* prtkimgdesc);
int check_secure_boot(struct t_rtkimgdesc* prtkimgdesc);
const char* get_kernel_RSA_sign_value(struct t_rtkimgdesc* prtkimgdesc, enum firm_ID eID);
const char* get_rootfs_RSA_sign_value(struct t_rtkimgdesc* prtkimgdesc);
const char* get_version(struct t_rtkimgdesc* prtkimgdesc);
int checkinstallbootcode(struct t_rtkimgdesc* prtkimgdesc);
int check_target_flash_is_spi(void);
#endif
