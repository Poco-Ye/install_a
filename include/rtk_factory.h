#ifndef RTK_FACTORY_H
#define RTK_FACTORY_H
#include <stdio.h>
#include "rtk_imgdesc.h"
int factory_init(const char* dir, struct t_rtkimgdesc* prtkimg);
int factory_load(const char* dir, struct t_rtkimgdesc* prtkimg);
int factory_flush(unsigned int factory_start, unsigned int factory_size, struct t_rtkimgdesc* prtkimg, bool bFlush=true);
const char *get_factory_tmp_dir(void);
const char get_factory_current_pp(void);
extern char factory_dir[32];
#endif
