#ifndef RTK_BURN_H
#define RTK_BURN_H

#include <rtk_boottable.h>

int modify_signature(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_bootcode(struct t_rtkimgdesc* prtkimgdesc);
int rtk_extract_file(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, const char* file_path);
int rtk_extract_utility(struct t_rtkimgdesc* prtkimgdesc);

int rtk_burn_img(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, int offset=0);
int rtk_burn_yaffs_img(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, struct t_PARTDESC* rtk_part);
int rtk_burn_jffs_img(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, struct t_PARTDESC* rtk_part);
int rtk_burn_ubifs_img(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, struct t_PARTDESC* rtk_part);
int rtk_burn_ext4_img(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc, struct t_PARTDESC* rtk_part, int offset=0);

#include <rtk_imgdesc.h>
int rtk_burn_single_part(struct t_rtkimgdesc* prtkimgdesc, enum FWTYPE efwtype);

int rtk_burn_etcimg(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc);
int rtk_burn_preserveimg(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc);

int rtk_burn_img_from_net(struct t_rtkimgdesc* prtkimgdesc, struct t_imgdesc* pimgdesc);
int rtk_burn_cipher_key(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_fwdesc(struct t_rtkimgdesc* prtkimgdesc,S_BOOTTABLE* pboottable=NULL);
int rtk_burn_bootcode_nand(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_bootcode_mac_spi(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_bootcode_mac_nand(struct t_rtkimgdesc* prtkimgdesc);
#ifdef EMMC_SUPPORT
#ifdef FLASH_OFFLINE_BIN_WRITER
#ifndef __OFFLINE_GENERATE_BIN__
int rtk_burn_flashbin(char* binFilePath, const char* target_devPath, int sizeCheck, FILE* customer_fp=NULL);
#endif
#endif
int rtk_burn_bootcode_emmc(struct t_rtkimgdesc* prtkimgdesc);
#endif
int rtk_install_factory(struct t_rtkimgdesc* prtkimgdesc, bool bFlush = true);
int rtk_flush_pingpong_factory_mac(struct t_rtkimgdesc* prtkimgdesc);
int rtk_create_remove_file_factory_mac(struct t_rtkimgdesc* prtkimgdesc, const char *filename, int mode);
int rtk_erase(struct t_rtkimgdesc* prtkimgdesc, unsigned int erase_start, unsigned int erase_length);
int rtk_check_update_bootcode(struct t_rtkimgdesc* prtkimgdesc);
#ifdef __OFFLINE_GENERATE_BIN__
int rtk_factory_to_virt_nand(struct t_rtkimgdesc* prtkimgdesc);
#endif
#endif
