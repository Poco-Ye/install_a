#ifndef RTK_CONFIG_H
#define RTK_CONFIG_H

#include <rtk_boottable.h>
#include <rtk_imgdesc.h>
FWTYPE etag_to_fwtype(E_TAG tag);
const char* inv_efs_to_str(eFS efs_type);
int rtk_load_config(struct t_rtkimgdesc* );


/* parse config yes:1   no:0 */
#define create_yn_fun(var_name) \
int fill_yn_##var_name(char* str, struct t_rtkimgdesc* prtkimg) \
{  \
   if(str == NULL||*str == 0) \
      return -1;  \
   if(str[0] == ' ') \
      str = skip_space(str);  \
   install_debug("%s(%s)(%d)\r\n", __func__, str, strlen(str)); \
   if((0 == strncmp(str, "y", 1)) || (0 == strncmp(str, "Y", 1)) || (0 == strncmp(str, "1", 1)))   \
   {  \
      prtkimg->var_name = 1;  \
      install_log("%s(y)\r\n", __func__); \
   }  \
   else  \
   {  \
      prtkimg->var_name = 0;  \
      install_log("%s(n)\r\n", __func__); \
   }  \
   return 0;   \
}
#define create_yn_match(var_name) \
if(0 == strncmp(#var_name, newline, strlen(#var_name))) \
{ \
   fill_yn_##var_name(skip_char(newline+strlen(#var_name), '='), prtkimg); \
   continue; \
}

/* parse config int */
#define create_var_fun(var_name) \
int fill_var_##var_name(char* str, struct t_rtkimgdesc* prtkimg) \
{  \
   char *str_end = NULL; \
   if(str == NULL||*str == 0) \
      return -1;  \
   if(str[0] == ' ') \
      str = skip_space(str);  \
   install_debug("%s(%s)(%lu)\r\n", __func__, str, strlen(str)); \
   if((0 == strncmp(str, "0x", 2)) || (0 == strncmp(str, "0X", 2)))   \
   {  \
      prtkimg->var_name = strtol(str, &str_end, 16); \
      install_log("%s(0x%x)\r\n", __func__, prtkimg->var_name); \
   }  \
   else  \
   {  \
      prtkimg->var_name = strtol(str, &str_end, 10); \
      install_log("%s(%d)\r\n", __func__, prtkimg->var_name); \
   }  \
   return 0;   \
}
#define create_var_match(var_name) \
if(0 == strncmp(#var_name, newline, strlen(#var_name))) \
{ \
   fill_var_##var_name(skip_char(newline+strlen(#var_name), '='), prtkimg); \
   continue; \
}

#endif
