/************************************************************************
 *
 *  external_param.h
 *
 *  external parameters format
 * 
 *
 ************************************************************************/

#ifndef EXTERN_PARAM_H
#define EXTERN_PARAM_H

#include <rtk_common.h>
/************************************************************************
 *  Include files
 ************************************************************************/

/************************************************************************
 *  Definitions
*************************************************************************/

typedef struct{
	unsigned int    secure_mode;
	unsigned int    flash_type;

	unsigned int    region;

	unsigned int    mac_hi;
	unsigned int    mac_lo;

	unsigned int 	  logo_img_saddr;
	unsigned int    logo_img_len;
	unsigned int    logo_type;
	unsigned int    logo_offset;
	unsigned int    logo_reg_5370;
	unsigned int    logo_reg_5374;
	unsigned int    logo_reg_5378;
	unsigned int    logo_reg_537c;

	unsigned int    rescue_img_size;
	unsigned int    rescue_img_addr;
	unsigned int    rescue_img_part0_saddr;
	unsigned int    rescue_img_part0_len;
	unsigned int    rescue_img_part1_saddr;
	unsigned int    rescue_img_part1_len;

	unsigned int    env_param_saddr;

	unsigned int    logo2_img_saddr;
	unsigned int    logo2_img_len;
	unsigned int    logo2_type;
	unsigned int    logo2_offset;
	unsigned int    logo2_reg_5370;
	unsigned int    logo2_reg_5374;
	unsigned int    logo2_reg_5378;
	unsigned int    logo2_reg_537c;

	unsigned int 	   logo3_img_saddr;
	unsigned int    logo3_img_len;
	unsigned int    logo3_type;
	unsigned int    logo3_offset;
	unsigned int    logo3_reg_5370;
	unsigned int    logo3_reg_5374;
	unsigned int    logo3_reg_5378;
	unsigned int    logo3_reg_537c;

	unsigned int    logo4_img_saddr;
	unsigned int    logo4_img_len;
	unsigned int    logo4_type;
	unsigned int    logo4_offset;
	unsigned int    logo4_reg_5370;
	unsigned int    logo4_reg_5374;
	unsigned int    logo4_reg_5378;
	unsigned int    logo4_reg_537c;

	unsigned int    array_img_saddr;
	unsigned int    array_img_size;

	// HDMI keys (additional one byte is used as string terminating symbol '\0')
	unsigned char   hdmi_key1[112+1];
	unsigned char   hdmi_key2[112+1];
	unsigned char   hdmi_key3[112+1];
	unsigned char   hdmi_key4[112+1];
	unsigned char   hdmi_key5[112+1];
	unsigned char   hdmi_key6[10+1];

	// customize value (additional one byte is used as string terminating symbol '\0')
	unsigned char   custom_field[32+1];

	unsigned int    logo5_img_saddr;
	unsigned int    logo5_img_len;
	unsigned int    logo5_type;
	unsigned int    logo5_offset;
	unsigned int    logo5_reg_5370;
	unsigned int    logo5_reg_5374;
	unsigned int    logo5_reg_5378;
	unsigned int    logo5_reg_537c;

	unsigned int    logo6_img_saddr;
	unsigned int    logo6_img_len;
	unsigned int    logo6_type;
	unsigned int    logo6_offset;
	unsigned int    logo6_reg_5370;
	unsigned int    logo6_reg_5374;
	unsigned int    logo6_reg_5378;
	unsigned int    logo6_reg_537c;

	unsigned int    logo7_img_saddr;
	unsigned int    logo7_img_len;
	unsigned int    logo7_type;
	unsigned int    logo7_offset;
	unsigned int    logo7_reg_5370;
	unsigned int    logo7_reg_5374;
	unsigned int    logo7_reg_5378;
	unsigned int    logo7_reg_537c;

	unsigned int    logo8_img_saddr;
	unsigned int    logo8_img_len;
	unsigned int    logo8_type;
	unsigned int    logo8_offset;
	unsigned int    logo8_reg_5370;
	unsigned int    logo8_reg_5374;
	unsigned int    logo8_reg_5378;
	unsigned int    logo8_reg_537c;

	unsigned int    bootcode_img_saddr;
	unsigned int    bootcode_img_size;

	unsigned int    data_start_blk;
} __attribute__((aligned(8))) t_extern_param;


//2012.08.16
//copy from NAND_bootcode_updater/extern_param.h
//strange it is a little bit different from spi t_extern_param
//so rename to t_extern_param_nand
typedef struct {

	unsigned int flash_type;		
	
	unsigned int region;
	
	unsigned int mac_hi;
	unsigned int mac_lo;
	
	unsigned char * logo_img_saddr;
	unsigned int logo_img_len;
	unsigned int logo_type;
	unsigned int logo_offset;
	unsigned int logo_reg_5370;
	unsigned int logo_reg_5374;
	unsigned int logo_reg_5378;
	unsigned int logo_reg_537c;
		
	unsigned int rescue_img_size;
	unsigned char * rescue_img_part0_saddr;
	unsigned int rescue_img_part0_len;
	unsigned char * rescue_img_part1_saddr;
	unsigned int rescue_img_part1_len;

	unsigned char * env_param_saddr;

	unsigned char * logo2_img_saddr;
	unsigned int logo2_img_len;
	unsigned int logo2_type;
	unsigned int logo2_offset;
	unsigned int logo2_reg_5370;
	unsigned int logo2_reg_5374;
	unsigned int logo2_reg_5378;
	unsigned int logo2_reg_537c;
	
	unsigned char * logo3_img_saddr;
	unsigned int logo3_img_len;
	unsigned int logo3_type;
	unsigned int logo3_offset;
	unsigned int logo3_reg_5370;
	unsigned int logo3_reg_5374;
	unsigned int logo3_reg_5378;
	unsigned int logo3_reg_537c;
	
	unsigned char * logo4_img_saddr;
	unsigned int logo4_img_len;
	unsigned int logo4_type;
	unsigned int logo4_offset;
	unsigned int logo4_reg_5370;
	unsigned int logo4_reg_5374;
	unsigned int logo4_reg_5378;
	unsigned int logo4_reg_537c;
	
	unsigned char * array_img_saddr;
	unsigned int array_img_size;
	
	// HDMI keys (additional one byte is used as string terminating symbol '\0')
	unsigned char hdmi_key1[112+1];
	unsigned char hdmi_key2[112+1];
	unsigned char hdmi_key3[112+1];
	unsigned char hdmi_key4[112+1];
	unsigned char hdmi_key5[112+1];
	unsigned char hdmi_key6[10+1];

	// customize value (additional one byte is used as string terminating symbol '\0')
	unsigned char custom_field[32+1];

	unsigned char * logo5_img_saddr;
	unsigned int logo5_img_len;
	unsigned int logo5_type;
	unsigned int logo5_offset;
	unsigned int logo5_reg_5370;
	unsigned int logo5_reg_5374;
	unsigned int logo5_reg_5378;
	unsigned int logo5_reg_537c;
	
	unsigned char * logo6_img_saddr;
	unsigned int logo6_img_len;
	unsigned int logo6_type;
	unsigned int logo6_offset;
	unsigned int logo6_reg_5370;
	unsigned int logo6_reg_5374;
	unsigned int logo6_reg_5378;
	unsigned int logo6_reg_537c;
	
	unsigned char * logo7_img_saddr;
	unsigned int logo7_img_len;
	unsigned int logo7_type;
	unsigned int logo7_offset;
	unsigned int logo7_reg_5370;
	unsigned int logo7_reg_5374;
	unsigned int logo7_reg_5378;
	unsigned int logo7_reg_537c;
	
	unsigned char * logo8_img_saddr;
	unsigned int logo8_img_len;
	unsigned int logo8_type;
	unsigned int logo8_offset;
	unsigned int logo8_reg_5370;
	unsigned int logo8_reg_5374;
	unsigned int logo8_reg_5378;
	unsigned int logo8_reg_537c;
	
	unsigned char * bootcode_img_saddr;
	unsigned int  bootcode_img_size;
	
} __attribute__((aligned(8))) t_extern_param_nand;
#endif //#ifndef EXTERN_PARAM_H	
