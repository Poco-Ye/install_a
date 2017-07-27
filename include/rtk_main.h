#ifndef RTK_MAIN_H
#define RTK_MAIN_H

#include <rtk_imgdesc.h>
#include <rtk_common.h>


#ifndef __OFFLINE_GENERATE_BIN__
#ifdef FLASH_OFFLINE_BIN_WRITER
int rtk_burn_offlineBin_from_usb(struct t_rtkimgdesc* prtkimgdesc);
#endif
#endif

int rtk_burn_rtkimg_from_usb(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_rtkimg_from_local_flash(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_rtkimg_from_urltar(struct t_rtkimgdesc* prtkimgdesc);
int rtk_burn_rtkimg_from_urltar_by_downloading_tarfile(struct t_rtkimgdesc* prtkimgdesc);

#ifdef __OFFLINE_GENERATE_BIN__
int rtk_offline_generate_bin(struct t_rtkimgdesc* prtkimgdesc);
#endif

#endif
