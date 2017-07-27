#include <string.h>

#include <rtk_common.h>
#include <rtk_burn.h>
#include <rtk_fwdesc.h>
#include <rtk_tagflow.h>
#include <rtk_imgdesc.h>
#include <rtk_boottable.h>
#include <rtk_mtd.h>
#include <rtk_customer.h>

#define SIZEOF_ARRAY(X)		(sizeof(X)/sizeof(X[0]))

extern u32 gDebugPrintfLogLevel;

struct fwMap {
	FWTYPE fwtype;
	E_FWTYPE e_type;
};

static fwMap audioFW[]= {
	{FW_AUDIO_BOOTFILE,FWTYPE_DALOGO},
	{FW_AUDIO_CLOGO1,FWTYPE_CALOGO1},
	{FW_AUDIO_CLOGO2,FWTYPE_CALOGO2},
};

static fwMap imageFW[]= {
	{FW_IMAGE_BOOTFILE,FWTYPE_DILOGO},
	{FW_IMAGE_CLOGO1,FWTYPE_CILOGO1},
	{FW_IMAGE_CLOGO2,FWTYPE_CILOGO2},
};

static fwMap videoFW[]= {
	{FW_VIDEO_BOOTFILE,FWTYPE_DVLOGO},
	{FW_VIDEO_CLOGO1,FWTYPE_CVLOGO1},
	{FW_VIDEO_CLOGO2,FWTYPE_CVLOGO2},
};

int tagflow3(S_BOOTTABLE* pboottable, struct t_rtkimgdesc* prtkimgdesc, S_BOOTTABLE* pbt)
{
   int ret = 0, etc_index = 0;
   //char cmd[128] = {0};
   enum FWTYPE efwtype;

   switch(pboottable->tag) {
      case TAG_UNKNOWN:
         pboottable->tag = TAG_UPDATE_ETC;
         break;

      case TAG_UPDATE_ETC:
         if (prtkimgdesc->fw[FW_USR_LOCAL_ETC].img_size == 0) {
            install_info("There is no USR_LOCAL_ETC partition\n");
            pboottable->tag = TAG_UPDATE_NL;
            break;
         }

#ifdef NAS_ENABLE
         if (prtkimgdesc->update_etc == 1 && prtkimgdesc->mode != _UPGRAD) {
#else
         if (prtkimgdesc->update_etc == 1) {
#endif
            // umount
            install_log("update_etc=y, start update etc\r\n");

#ifndef __OFFLINE_GENERATE_BIN__
            //snprintf(cmd, sizeof(cmd), "umount %s", prtkimgdesc->fw[FW_USR_LOCAL_ETC].mount_point);
            //rtk_command(cmd, __LINE__, __FILE__);
#endif
            if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
               remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
               add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC] );
            }
            else {
               install_fail("Burn ETC fail\r\n");
               return -_eRTK_TAG_UPDATE_ETC_FAIL;
            }
         }
         else {
            //check if the etc partition is already in boottable. if not, try to burn ETC.
            if ((etc_index = get_index_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name)) < 0) {
               //etc partition is not in boottable.
               install_log("update_etc=n, but cannot find etc partition in boottable, start burn etc\r\n");
               // force update etc
               prtkimgdesc->update_etc = 1;
               if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
                  remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
                  add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC] );
               }
               else {
                  install_fail("Burn ETC fail\r\n");
                  return -_eRTK_TAG_UPDATE_ETC_FAIL;
               }
            }
            else if (((pboottable->part.list[etc_index].loc.offset != prtkimgdesc->fw[FW_USR_LOCAL_ETC].flash_offset) ||
               (pboottable->part.list[etc_index].loc.size != prtkimgdesc->fw[FW_USR_LOCAL_ETC].flash_allo_size)) &&
               prtkimgdesc->flash_type != MTD_NANDFLASH) {
               install_log("update_etc=n, move partition\r\n");
               if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
                  remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
                  add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC]);
               }
               else {
                  install_fail("Burn ETC fail\r\n");
                  return -_eRTK_TAG_UPDATE_ETC_FAIL;
               }
            }
            else {
               install_log("update_etc=n, skip update etc\r\n");
            }
         }
         pboottable->tag = TAG_UPDATE_NL;
         break;

      case TAG_UPDATE_NL:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL])) == 0) {
            update_fw(pboottable, FWTYPE_KERNEL, &prtkimgdesc->fw[FW_KERNEL]);
         }
         else {
            install_fail("Burn KERNEL fail\r\n");
            return -_eRTK_TAG_UPDATE_NL_FAIL;
         }
         pboottable->tag = TAG_UPDATE_RL_DT;
         break;

	  case TAG_UPDATE_RL_DT:
	  	 if( prtkimgdesc->install_dtb == 1 ) {
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_DT])) == 0) {
	            update_fw(pboottable, FWTYPE_RESCUE_DT, &prtkimgdesc->fw[FW_RESCUE_DT]);
	         }
	         else {
	            install_fail("Burn RSCUE_DT fail\r\n");
	            return -_eRTK_TAG_UPDATE_RL_DT_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_NL_DT;
         break;

	  case TAG_UPDATE_NL_DT:
	  	 if( prtkimgdesc->install_dtb == 1 ) {
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_DT])) == 0) {
	            update_fw(pboottable, FWTYPE_KERNEL_DT, &prtkimgdesc->fw[FW_KERNEL_DT]);
	         }
	         else {
	            install_fail("Burn KERNEL_DT fail\r\n");
	            return -_eRTK_TAG_UPDATE_NL_DT_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_RL_ROOTFS;
         break;

	  case TAG_UPDATE_RL_ROOTFS:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_RESCUE_ROOTFS]);
         }
         else {
            install_fail("Burn RESCUE_ROOTFS fail\r\n");
            return -_eRTK_TAG_UPDATE_RL_ROOTFS_FAIL;
         }
#ifdef NAS_ENABLE
         if (1 == prtkimgdesc->nas_rescue)
         pboottable->tag = TAG_UPDATE_NAS_NL;
         else
#endif
         pboottable->tag = TAG_UPDATE_NL_ROOTFS;
         break;

#ifdef NAS_ENABLE
	  case TAG_UPDATE_NAS_NL:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_KERNEL])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_KERNEL, &prtkimgdesc->fw[FW_NAS_KERNEL]);
         }
         else {
            install_fail("Burn NAS rescue kernel fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_NL_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NAS_RL_DT;
         break;

	  case TAG_UPDATE_NAS_RL_DT:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_RESCUE_DT])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_DT, &prtkimgdesc->fw[FW_NAS_RESCUE_DT]);
         }
         else {
            install_fail("Burn NAS rescue DTB fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_RL_DT_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NAS_RL_ROOTFS;
         break;

          case TAG_UPDATE_NAS_RL_ROOTFS:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_RESCUE_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_NAS_RESCUE_ROOTFS]);
         }
         else {
            install_fail("Burn NAS rescue rootfs fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_RL_ROOTFS_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NL_ROOTFS;
         break;

#endif
	  case TAG_UPDATE_NL_ROOTFS:
#if defined(NAS_ENABLE) || defined(GENERIC_LINUX)
		 if (prtkimgdesc->fw[FW_KERNEL_ROOTFS].img_size == 0) {
			install_info("There is no KERNEL_ROOTFS partition\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_KERNEL_ROOTFS, &prtkimgdesc->fw[FW_KERNEL_ROOTFS]);
         }
         else {
            install_fail("Burn KERNEL_ROOTFS fail\r\n");
            return -_eRTK_TAG_UPDATE_NL_ROOTFS_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_TEE;
         break;

      case TAG_UPDATE_TEE:
#ifdef TEE_ENABLE
#if 1//def NAS_ENABLE
		 if (prtkimgdesc->fw[FW_TEE].img_size == 0) {
			install_info("There is no TEE FW\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_TEE])) == 0) {
            update_fw(pboottable, FWTYPE_TEEKERNEL, &prtkimgdesc->fw[FW_TEE]);
         }
         else {
            install_fail("Burn TEE FW fail\r\n");
            return -_eRTK_TAG_UPDATE_TEEK_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_BL31;
         break;
         
      case TAG_UPDATE_BL31:
#if 1//def NAS_ENABLE
         if (prtkimgdesc->fw[FW_BL31].img_size == 0) {
            install_info("There is no BL31 FW\n");
         }
         else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_BL31])) == 0) {
            update_fw(pboottable, FWTYPE_BL31, &prtkimgdesc->fw[FW_BL31]);
         }
         else {
            install_fail("Burn BL31 FW fail\r\n");
            return -_eRTK_TAG_UPDATE_BL31_FAIL;
         }
         }
#endif
         pboottable->tag = TAG_UPDATE_XEN;
         break;

      case TAG_UPDATE_XEN: 
#ifdef NAS_ENABLE
#ifdef HYPERVISOR
         if (prtkimgdesc->fw[FW_XEN].img_size == 0)
         {
            install_info("There is no XEN_OS image.\n");
         }
         else
         {
             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_XEN])) == 0) {
                update_fw(pboottable, FWTYPE_XEN, &prtkimgdesc->fw[FW_XEN]);
             }
             else {
                install_fail("Burn XEN_OS fail\r\n");
                return -_eRTK_TAG_UPDATE_XEN_FAIL;
             }
         }
#endif
#endif
         pboottable->tag = TAG_UPDATE_AK;
         break;
         
      case TAG_UPDATE_AK:
#ifdef NAS_ENABLE
		 if (prtkimgdesc->fw[FW_AUDIO].img_size == 0) {
			install_info("There is no AUDIO FW\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_AUDIO])) == 0) {
            update_fw(pboottable, FWTYPE_AKERNEL, &prtkimgdesc->fw[FW_AUDIO]);
         }
         else {
            install_fail("Burn AUDIO FW fail\r\n");
            return -_eRTK_TAG_UPDATE_AK_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_DALOGO;
         break;

      /*
      case TAG_UPDATE_VK:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_VIDEO])) == 0) {
            update_fw(pboottable, FWTYPE_VKERNEL   \
            , prtkimgdesc->fw[FW_VIDEO].mem_offset \
            , prtkimgdesc->fw[FW_VIDEO].flash_offset \
            , prtkimgdesc->fw[FW_VIDEO].img_size \
            , prtkimgdesc->fw[FW_VIDEO].compress_type);
         }
         else {
            install_fail("Burn VIDEO FW fail\r\n");
            return -_eRTK_TAG_UPDATE_VK_FAIL;
         }
         pboottable->tag = TAG_UPDATE_DALOGO;
         break;
      */
      case TAG_UPDATE_DALOGO:
         for( unsigned int i=0; i<SIZEOF_ARRAY(audioFW); i++ )
         {
             if ( prtkimgdesc->fw[ audioFW[i].fwtype ].img_size == 0 ) {
                 install_info("There are no Audio bootfile(%d)\n", i);
                 update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
                 continue;
             }

             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ audioFW[i].fwtype ])) == 0) {
                update_fw(pboottable, audioFW[i].e_type, &prtkimgdesc->fw[ audioFW[i].fwtype ]);
             }
             else {
                install_debug("Burn AUDIO LOGO(%d) fail\nSet DALOGO target, offset & size to 0\n", i);
                update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
             }
         }
         pboottable->tag = TAG_UPDATE_DILOGO;
         break;

      case TAG_UPDATE_DILOGO:
         for( unsigned int i=0; i<SIZEOF_ARRAY(imageFW); i++ )
         {
             if ( prtkimgdesc->fw[ imageFW[i].fwtype ].img_size == 0 ) {
                 install_info("There are no Image bootfile(%d)\n", i);
                 update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
                 continue;
             }

             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ imageFW[i].fwtype ])) == 0) {
                update_fw(pboottable, imageFW[i].e_type, &prtkimgdesc->fw[ imageFW[i].fwtype ]);
             }
             else {
                install_debug("Burn IMAGE LOGO(%d) fail\nSet DILOGO target, offset & size to 0\n",i);
                update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
             }
         }
         pboottable->tag = TAG_UPDATE_DVLOGO;
         break;

      case TAG_UPDATE_DVLOGO:
         for( unsigned int i=0; i<SIZEOF_ARRAY(videoFW); i++ )
         {
             if ( prtkimgdesc->fw[ videoFW[i].fwtype ].img_size == 0 ) {
                install_info("There are no Video bootfile(%d)\n", i);
                update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
                continue;
             }

             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ videoFW[i].fwtype ])) == 0) {
                update_fw(pboottable, videoFW[i].e_type, &prtkimgdesc->fw[ videoFW[i].fwtype ]);
             }
             else {
                install_debug("Burn VIDEO LOGO(%d) fail\nSet DVLOGO target, offset & size to 0\n",i);
                update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
             }
         }
         pboottable->tag = TAG_UPDATE_ROOTFS;
         break;

      case TAG_UPDATE_ROOTFS:
		 if (prtkimgdesc->fw[FW_ROOTFS].img_size == 0) {
			install_info("There is no ROOTFS partition\n");
		 }
		 else {
         	if ((ret = rtk_burn_single_part(prtkimgdesc, FW_ROOTFS)) == 0) {
            	remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_ROOTFS].part_name);
            	add_part(pboottable, &prtkimgdesc->fw[FW_ROOTFS]);
         	}
         	else {
            	install_fail("Burn ROOTFS fail\r\n");
         	}
		 }
         pboottable->tag = TAG_UPDATE_RES;
         break;

	  case TAG_UPDATE_RES:
		if (prtkimgdesc->fw[FW_RES].img_size == 0) {
			install_info("There is no RES partition\n");
			pboottable->tag = TAG_UPDATE_CACHE;
			break;
		}

		 if ((ret = rtk_burn_single_part(prtkimgdesc, FW_RES)) == 0) {
			remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_RES].part_name);
			add_part(pboottable, &prtkimgdesc->fw[FW_RES]);
		 }
		 else {
			install_fail("Burn RES fail\r\n");
			return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
		 }
		 pboottable->tag = TAG_UPDATE_CACHE;
		 break;

	  case TAG_UPDATE_CACHE:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_CACHE].img_size == 0) {
			install_info("There is no CACHE partition\n");
			pboottable->tag = TAG_UPDATE_DATA;
			break;
		}
#endif
		 if ((ret = rtk_burn_single_part(prtkimgdesc, FW_CACHE)) == 0) {
			remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_CACHE].part_name);
			add_part(pboottable, &prtkimgdesc->fw[FW_CACHE]);
		 }
		 else {
			install_fail("Burn CACHE fail\r\n");
			return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
		 }
		 pboottable->tag = TAG_UPDATE_DATA;
		 break;

	  case TAG_UPDATE_DATA:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_DATA].img_size == 0) {
			install_info("There is no DATA partition\n");
			pboottable->tag = TAG_UPDATE_SYSTEM;
			break;
		}
#endif
		 if ((ret = rtk_burn_single_part(prtkimgdesc, FW_DATA)) == 0) {
			remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_DATA].part_name);
			add_part(pboottable, &prtkimgdesc->fw[FW_DATA]);
		 }
		 else {
			install_fail("Burn DATA fail\r\n");
			return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
		 }
		 pboottable->tag = TAG_UPDATE_SYSTEM;
		 break;

	  case TAG_UPDATE_SYSTEM:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_SYSTEM].img_size == 0) {
			install_info("There is no SYSTEM partition\n");
			pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
			break;
		}
#endif
		 if ((ret = rtk_burn_single_part(prtkimgdesc, FW_SYSTEM)) == 0) {
			remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_SYSTEM].part_name);
			add_part(pboottable, &prtkimgdesc->fw[FW_SYSTEM]);
		 }
		 else {
			install_fail("Burn SYSTEM fail\r\n");
			return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
		 }
		 pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
		 break;

      case TAG_UPDATE_OTHER_PARTITION1:
         for (efwtype = P_PARTITION1; efwtype <= P_PARTITION8; efwtype=FWTYPE(efwtype+1)) {
            if (prtkimgdesc->fw[efwtype].flash_allo_size != 0 && prtkimgdesc->fw[efwtype].img_size!=0 && strlen(prtkimgdesc->fw[efwtype].part_name) != 0) {
	            if ((ret = rtk_burn_single_part(prtkimgdesc, efwtype)) == 0) {
#ifdef NAS_ENABLE
	               remove_part_by_partname(pboottable, prtkimgdesc->fw[efwtype].part_name);
#endif
	               add_part(pboottable, &prtkimgdesc->fw[efwtype]);
	            } else {
	               install_fail("Burn Partition(%s) fail\r\n", prtkimgdesc->fw[efwtype].part_name );
	               return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
	            }
            }
         }

#if 0
         if (prtkimgdesc->fw[FW_AUDIO_CLOGO1].flash_allo_size != 0)
            update_fw(pboottable, FWTYPE_CALOGO1 \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO1].mem_offset \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO1].flash_offset \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO1].flash_allo_size\
            , NULL);
         else
            update_fw(pboottable, FWTYPE_CALOGO1 \
            , 0 \
            , 0 \
            , 0 \
            , NULL);

         if (prtkimgdesc->fw[FW_VIDEO_CLOGO1].flash_allo_size != 0)
            update_fw(pboottable, FWTYPE_CVLOGO1 \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO1].mem_offset \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO1].flash_offset \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO1].flash_allo_size\
            , NULL);
         else
            update_fw(pboottable, FWTYPE_CVLOGO1 \
            , 0 \
            , 0 \
            , 0 \
            , NULL);

         if (prtkimgdesc->fw[FW_AUDIO_CLOGO2].flash_allo_size != 0)
            update_fw(pboottable, FWTYPE_CALOGO2 \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO2].mem_offset \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO2].flash_offset \
            , prtkimgdesc->fw[FW_AUDIO_CLOGO2].flash_allo_size\
            , NULL);
         else
            update_fw(pboottable, FWTYPE_CALOGO2 \
            , 0 \
            , 0 \
            , 0 \
            , NULL);

         if (prtkimgdesc->fw[FW_VIDEO_CLOGO2].flash_allo_size != 0)
            update_fw(pboottable, FWTYPE_CVLOGO2 \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO2].mem_offset \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO2].flash_offset \
            , prtkimgdesc->fw[FW_VIDEO_CLOGO2].flash_allo_size\
            , NULL);
         else
            update_fw(pboottable, FWTYPE_CVLOGO2 \
            , 0 \
            , 0 \
            , 0 \
            , NULL);
#endif
         //update SSUWORKPART
         update_ssu_work_part(pboottable, prtkimgdesc->next_ssu_work_part);

         pboottable->tag = TAG_COMPLETE;
         pboottable->boottype = BOOTTYPE_COMPLETE;
         break;

      case TAG_COMPLETE:
         pboottable->tag = TAG_UPDATE_ETC;
         break;

      default:
         install_test("Error! should not happened TAG:%d\r\n", pboottable->tag);
   }

   return _eRTK_SUCCESS;
}

#ifdef EMMC_SUPPORT
int tagflow3_emmc(S_BOOTTABLE* pboottable, struct t_rtkimgdesc* prtkimgdesc, S_BOOTTABLE* pbt)
{
   int ret = 0;
   //char cmd[128] = {0};
   enum FWTYPE efwtype;

   switch(pboottable->tag) {
      case TAG_UNKNOWN:
         pboottable->tag = TAG_UPDATE_ETC;
         break;

      case TAG_UPDATE_ETC:
		if (prtkimgdesc->fw[FW_USR_LOCAL_ETC].img_size == 0) {
			install_info("There is no USR_LOCAL_ETC partition\n");
		}
		else {
      		if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
        		remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
            	add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC]);
        	}
        	else {
        		install_fail("Burn ETC fail\r\n");
        	}
		}
#ifndef NFLASH_LAOUT
        pboottable->tag = TAG_UPDATE_NL;
        break;
#else
        pboottable->tag = TAG_UPDATE_GOLD_NL;
        break;
		
      case TAG_UPDATE_GOLD_NL:
	  	 if (prtkimgdesc->fw[FW_GOLD_KERNEL].img_size != 0){
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_KERNEL])) == 0) {
	            update_fw(pboottable, FWTYPE_GOLD_KERNEL, &prtkimgdesc->fw[FW_GOLD_KERNEL]);
	         }
	         else {
	            install_fail("Burn FW_GOLD_KERNEL fail\r\n");
	            return -_eRTK_TAG_UPDATE_G_NL_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_GOLD_RL_DT;
         break;

      case TAG_UPDATE_GOLD_RL_DT:
	  	 if (prtkimgdesc->fw[FW_GOLD_RESCUE_DT].img_size != 0){
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_RESCUE_DT])) == 0) {
	            update_fw(pboottable, FWTYPE_GOLD_RESCUE_DT, &prtkimgdesc->fw[FW_GOLD_RESCUE_DT]);
	         }
	         else {
	            install_fail("Burn FW_GOLD_RESCUE_DT fail\r\n");
	            return -_eRTK_TAG_UPDATE_G_RL_DT_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_GOLD_RL_ROOTFS;
         break;

      case TAG_UPDATE_GOLD_RL_ROOTFS:
	  	 if (prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS].img_size != 0){
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS])) == 0) {
				update_fw(pboottable, FWTYPE_GOLD_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS]);
	         }
	         else {
				install_fail("Burn FW_GOLD_RESCUE_ROOTFS fail\r\n");
				return -_eRTK_TAG_UPDATE_G_RL_ROOTFS_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_GOLD_AK;
         break;
		 
      case TAG_UPDATE_GOLD_AK:
	  	 if (prtkimgdesc->fw[FW_GOLD_AUDIO].img_size != 0){
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_AUDIO])) == 0) {
				update_fw(pboottable, FWTYPE_GOLD_AKERNEL, &prtkimgdesc->fw[FW_GOLD_AUDIO]);
	         }
	         else {
				install_fail("Burn FW_GOLD_AUDIO fail\r\n");
				return -_eRTK_TAG_UPDATE_G_AK_FAIL;
	         }
	  	 } 
         pboottable->tag = TAG_UPDATE_NL;
         break;
#endif
      case TAG_UPDATE_NL:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL])) == 0) {
            update_fw(pboottable, FWTYPE_KERNEL, &prtkimgdesc->fw[FW_KERNEL]);
         }
         else {
            install_fail("Burn KERNEL fail\r\n");
            return -_eRTK_TAG_UPDATE_NL_FAIL;
         }
         pboottable->tag = TAG_UPDATE_RL_DT;
         break;

      case TAG_UPDATE_RL_DT:
	  	 if( prtkimgdesc->install_dtb == 1 ) {
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_DT])) == 0) {
	            update_fw(pboottable, FWTYPE_RESCUE_DT, &prtkimgdesc->fw[FW_RESCUE_DT]);
	         }
	         else {
	            install_fail("Burn RSCUE_DT fail\r\n");
	            return -_eRTK_TAG_UPDATE_RL_DT_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_NL_DT;
         break;

      case TAG_UPDATE_NL_DT:
 	  	 if( prtkimgdesc->install_dtb == 1 ) {
	         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_DT])) == 0) {
	            update_fw(pboottable, FWTYPE_KERNEL_DT, &prtkimgdesc->fw[FW_KERNEL_DT]);
	         }
	         else {
	            install_fail("Burn KERNEL_DT fail\r\n");
	            return -_eRTK_TAG_UPDATE_NL_DT_FAIL;
	         }
 	  	 }
         pboottable->tag = TAG_UPDATE_RL_ROOTFS;
         break;

      case TAG_UPDATE_RL_ROOTFS:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_RESCUE_ROOTFS]);
         }
         else {
            install_fail("Burn RESCUE_ROOTFS fail\r\n");
            return -_eRTK_TAG_UPDATE_RL_ROOTFS_FAIL;
         }
#ifdef NAS_ENABLE
         if (1 == prtkimgdesc->nas_rescue)
         pboottable->tag = TAG_UPDATE_NAS_NL;
         else
#endif
         pboottable->tag = TAG_UPDATE_NL_ROOTFS;
         break;

#ifdef NAS_ENABLE
	  case TAG_UPDATE_NAS_NL:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_KERNEL])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_KERNEL, &prtkimgdesc->fw[FW_NAS_KERNEL]);
         }
         else {
            install_fail("Burn NAS rescue kernel fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_NL_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NAS_RL_DT;
         break;

	  case TAG_UPDATE_NAS_RL_DT:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_RESCUE_DT])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_DT, &prtkimgdesc->fw[FW_NAS_RESCUE_DT]);
         }
         else {
            install_fail("Burn NAS rescue DTB fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_RL_DT_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NAS_RL_ROOTFS;
         break;

          case TAG_UPDATE_NAS_RL_ROOTFS:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_NAS_RESCUE_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_NAS_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_NAS_RESCUE_ROOTFS]);
         }
         else {
            install_fail("Burn NAS rescue rootfs fail\r\n");
            return -_eRTK_TAG_UPDATE_NAS_RL_ROOTFS_FAIL;
         }
         pboottable->tag = TAG_UPDATE_NL_ROOTFS;
         break;

#endif

      case TAG_UPDATE_NL_ROOTFS:
#if defined(NAS_ENABLE) || defined(GENERIC_LINUX)
		 if (prtkimgdesc->fw[FW_KERNEL_ROOTFS].img_size == 0) {
			install_info("There is no KERNEL_ROOTFS partition\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_ROOTFS])) == 0) {
            update_fw(pboottable, FWTYPE_KERNEL_ROOTFS, &prtkimgdesc->fw[FW_KERNEL_ROOTFS]);
         }
         else {
            install_fail("Burn KERNEL_ROOTFS fail\r\n");
            return -_eRTK_TAG_UPDATE_NL_ROOTFS_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_TEE;
         break;

      case TAG_UPDATE_TEE:
#ifdef TEE_ENABLE
#if 1//def NAS_ENABLE
		 if (prtkimgdesc->fw[FW_TEE].img_size == 0) {
			install_info("There is no TEE FW\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_TEE])) == 0) {
            update_fw(pboottable, FWTYPE_TEEKERNEL, &prtkimgdesc->fw[FW_TEE]);
         }
         else {
            install_fail("Burn TEE FW fail\r\n");
            return -_eRTK_TAG_UPDATE_TEEK_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_BL31;
         break;

      case TAG_UPDATE_BL31:
#if 1//def NAS_ENABLE
         if (prtkimgdesc->fw[FW_BL31].img_size == 0) {
            install_info("There is no BL31 FW\n");
         }
         else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_BL31])) == 0) {
            update_fw(pboottable, FWTYPE_BL31, &prtkimgdesc->fw[FW_BL31]);
         }
         else {
            install_fail("Burn BL31 FW fail\r\n");
            return -_eRTK_TAG_UPDATE_BL31_FAIL;
         }
         }
#endif
         pboottable->tag = TAG_UPDATE_XEN;
         break;

      case TAG_UPDATE_XEN: 
#ifdef NAS_ENABLE
#ifdef HYPERVISOR
         if (prtkimgdesc->fw[FW_XEN].img_size == 0)
         {
            install_info("There is no XEN_OS image.\n");
         }
         else
         {
             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_XEN])) == 0) {
                update_fw(pboottable, FWTYPE_XEN, &prtkimgdesc->fw[FW_XEN]);
             }
             else {
                install_fail("Burn XEN_OS fail\r\n");
                return -_eRTK_TAG_UPDATE_XEN_FAIL;
             }
         }
#endif
#endif
         pboottable->tag = TAG_UPDATE_AK;
         break;
         
      case TAG_UPDATE_AK:
#ifdef NAS_ENABLE
		 if (prtkimgdesc->fw[FW_AUDIO].img_size == 0) {
			install_info("There is no AUDIO FW\n");
		 }
		 else
#endif
         {
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_AUDIO])) == 0) {
            update_fw(pboottable, FWTYPE_AKERNEL, &prtkimgdesc->fw[FW_AUDIO]);
         }
         else {
            install_fail("Burn AUDIO FW fail\r\n");
            return -_eRTK_TAG_UPDATE_AK_FAIL;
         }
         }
         pboottable->tag = TAG_UPDATE_DALOGO;
         break;

#if 0
      case TAG_UPDATE_VK:
         if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_VIDEO])) == 0) {
            update_fw(pboottable, FWTYPE_VKERNEL   \
            , prtkimgdesc->fw[FW_VIDEO].mem_offset \
            , prtkimgdesc->fw[FW_VIDEO].flash_offset \
            , prtkimgdesc->fw[FW_VIDEO].img_size \
            , prtkimgdesc->fw[FW_VIDEO].compress_type);
         }
         else {
            install_fail("Burn VIDEO FW fail\r\n");
            return -_eRTK_TAG_UPDATE_VK_FAIL;
         }
         pboottable->tag = TAG_UPDATE_VK2;
         break;
	  case TAG_UPDATE_VK2:
		 if (prtkimgdesc->fw[FW_VIDEO2].img_size == 0) {
			install_info("There is no video2 FW\n");
		 }
		 else {
         	if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_VIDEO2])) == 0) {
            	update_fw(pboottable, FWTYPE_VKERNEL2   \
            	, prtkimgdesc->fw[FW_VIDEO2].mem_offset \
            	, prtkimgdesc->fw[FW_VIDEO2].flash_offset \
            	, prtkimgdesc->fw[FW_VIDEO2].img_size \
            	, prtkimgdesc->fw[FW_VIDEO2].compress_type);
         	}
         	else {
            	install_fail("Burn VIDEO2 FW fail\r\n");
         	}
		 }
         pboottable->tag = TAG_UPDATE_ECPUK;
         break;
	  case TAG_UPDATE_ECPUK:
		 if (prtkimgdesc->fw[FW_ECPU].img_size == 0) {
			install_info("There is no ECPU FW\n");
		 }
		 else {
         	if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_ECPU])) == 0) {
            	update_fw(pboottable, FWTYPE_ECPU   \
            	, prtkimgdesc->fw[FW_ECPU].mem_offset \
            	, prtkimgdesc->fw[FW_ECPU].flash_offset \
            	, prtkimgdesc->fw[FW_ECPU].img_size \
            	, prtkimgdesc->fw[FW_ECPU].compress_type);
         	}
         	else {
            	install_fail("Burn ECPU FW fail\r\n");
         	}
		 }
         pboottable->tag = TAG_UPDATE_DALOGO;
         break;
#endif
	case TAG_UPDATE_DALOGO:
	   for( unsigned int i=0; i<SIZEOF_ARRAY(audioFW); i++ )
	   {
		   if ( prtkimgdesc->fw[ audioFW[i].fwtype ].img_size == 0 ) {
			   install_info("There are no Audio bootfile(%d)\n", i);
			   update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
			   continue;
		   }

		   if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ audioFW[i].fwtype ])) == 0) {
			  update_fw(pboottable, audioFW[i].e_type, &prtkimgdesc->fw[ audioFW[i].fwtype ]);
		   }
		   else {
			  install_debug("Burn AUDIO LOGO(%d) fail\nSet DALOGO target, offset & size to 0\n", i);
			  update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
		   }
	   }
	   pboottable->tag = TAG_UPDATE_DILOGO;
	   break;

	case TAG_UPDATE_DILOGO:
	   for( unsigned int i=0; i<SIZEOF_ARRAY(imageFW); i++ )
	   {
		   if ( prtkimgdesc->fw[ imageFW[i].fwtype ].img_size == 0 ) {
			   install_info("There are no Image bootfile(%d)\n", i);
			   update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
			   continue;
		   }

		   if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ imageFW[i].fwtype ])) == 0) {
			  update_fw(pboottable, imageFW[i].e_type, &prtkimgdesc->fw[ imageFW[i].fwtype ]);
		   }
		   else {
			  install_debug("Burn IMAGE LOGO(%d) fail\nSet DILOGO target, offset & size to 0\n",i);
			  update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
		   }
	   }
	   pboottable->tag = TAG_UPDATE_DVLOGO;
	   break;

	case TAG_UPDATE_DVLOGO:
	   for( unsigned int i=0; i<SIZEOF_ARRAY(videoFW); i++ )
	   {
		   if ( prtkimgdesc->fw[ videoFW[i].fwtype ].img_size == 0 ) {
			  install_info("There are no Video bootfile(%d)\n", i);
			  update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
			  continue;
		   }

		   if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ videoFW[i].fwtype ])) == 0) {
			  update_fw(pboottable, videoFW[i].e_type, &prtkimgdesc->fw[ videoFW[i].fwtype ]);
		   }
		   else {
			  install_debug("Burn VIDEO LOGO(%d) fail\nSet DVLOGO target, offset & size to 0\n",i);
			  update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
		   }
	   }
	   pboottable->tag = TAG_UPDATE_ROOTFS;
	   break;

      case TAG_UPDATE_ROOTFS:
		 if (prtkimgdesc->fw[FW_ROOTFS].img_size == 0) {
			install_info("There is no ROOTFS partition\n");
		 }
		 else {
         	if ((ret = rtk_burn_single_part(prtkimgdesc, FW_ROOTFS)) == 0) {
            	remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_ROOTFS].part_name);
            	add_part(pboottable, &prtkimgdesc->fw[FW_ROOTFS]);
         	}
         	else {
            	install_fail("Burn ROOTFS fail\r\n");
         	}
		 }
         pboottable->tag = TAG_UPDATE_RES;
         break;

      case TAG_UPDATE_RES:
        if (prtkimgdesc->fw[FW_RES].img_size == 0) {
            install_info("There is no RES partition\n");
            pboottable->tag = TAG_UPDATE_CACHE;
            break;
        }

         if ((ret = rtk_burn_single_part(prtkimgdesc, FW_RES)) == 0) {
            remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_RES].part_name);
            add_part(pboottable, &prtkimgdesc->fw[FW_RES]);
         }
         else {
            install_fail("Burn RES fail\r\n");
            return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
         }
         pboottable->tag = TAG_UPDATE_CACHE;
         break;

      case TAG_UPDATE_CACHE:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_CACHE].img_size == 0) {
			install_info("There is no CACHE partition\n");
			pboottable->tag = TAG_UPDATE_DATA;
			break;
		}
#endif
         if ((ret = rtk_burn_single_part(prtkimgdesc, FW_CACHE)) == 0) {
            remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_CACHE].part_name);
            add_part(pboottable, &prtkimgdesc->fw[FW_CACHE]);
         }
         else {
            install_fail("Burn CACHE fail\r\n");
            return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
         }
         pboottable->tag = TAG_UPDATE_DATA;
         break;

      case TAG_UPDATE_DATA:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_DATA].img_size == 0) {
			install_info("There is no DATA partition\n");
			pboottable->tag = TAG_UPDATE_SYSTEM;
			break;
		}
#endif
         if ((ret = rtk_burn_single_part(prtkimgdesc, FW_DATA)) == 0) {
            remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_DATA].part_name);
            add_part(pboottable, &prtkimgdesc->fw[FW_DATA]);
         }
         else {
            install_fail("Burn DATA fail\r\n");
            return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
         }
         pboottable->tag = TAG_UPDATE_SYSTEM;
         break;

      case TAG_UPDATE_SYSTEM:
#ifdef NAS_ENABLE
		if (prtkimgdesc->fw[FW_SYSTEM].img_size == 0) {
			install_info("There is no SYSTEM partition\n");
			pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
			break;
		}
#endif
         if ((ret = rtk_burn_single_part(prtkimgdesc, FW_SYSTEM)) == 0) {
            remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_SYSTEM].part_name);
            add_part(pboottable, &prtkimgdesc->fw[FW_SYSTEM]);
         }
         else {
            install_fail("Burn SYSTEM fail\r\n");
            return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
         }
#ifndef NFLASH_LAOUT
         pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
         break;
#else
         pboottable->tag = TAG_UPDATE_UBOOT;
         break;
		 
      case TAG_UPDATE_UBOOT:
	  	 if (prtkimgdesc->fw[FW_UBOOT].img_size != 0){
		     if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_UBOOT])) == 0) {
				update_fw(pboottable, FWTYPE_P_UBOOT, &prtkimgdesc->fw[FW_UBOOT]);
	         }
	         else {
	            install_fail("Burn FW_UBOOT fail\r\n");
	            return -_eRTK_TAG_UPDATE_UBOOT_FAIL;
	         }
	  	 }
         pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
         break;
#endif
      case TAG_UPDATE_OTHER_PARTITION1:
         for (efwtype = P_PARTITION1; efwtype <= P_PARTITION8; efwtype=FWTYPE(efwtype+1)) {
            if (prtkimgdesc->fw[efwtype].flash_allo_size != 0 && prtkimgdesc->fw[efwtype].img_size!=0 && strlen(prtkimgdesc->fw[efwtype].part_name) != 0) {
#ifndef PC_SIMULATE
				if (gDebugPrintfLogLevel&INSTALL_MEM_LEVEL)
					system("cat /proc/meminfo");
#endif
				if ((strcmp(prtkimgdesc->fw[efwtype].part_name, "uboot") == 0) 
					|| (strcmp(prtkimgdesc->fw[efwtype].part_name, "logo") == 0)){
#ifdef NAS_ENABLE
					remove_part_by_partname(pboottable, prtkimgdesc->fw[efwtype].part_name);
#endif
					add_part(pboottable, &prtkimgdesc->fw[efwtype]);
				}
				else{
					if ((ret = rtk_burn_single_part(prtkimgdesc, efwtype)) == 0) {
#ifdef NAS_ENABLE
						remove_part_by_partname(pboottable, prtkimgdesc->fw[efwtype].part_name);
#endif
	               		add_part(pboottable, &prtkimgdesc->fw[efwtype]);
	            	} else {
			            install_fail("Burn Partition(%s) fail\r\n", prtkimgdesc->fw[efwtype].part_name );
						return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
	            	}
				}
			}
         }

         //update SSUWORKPART
         update_ssu_work_part(pboottable, prtkimgdesc->next_ssu_work_part);

         pboottable->tag = TAG_COMPLETE;
         pboottable->boottype = BOOTTYPE_COMPLETE;
         break;

      case TAG_COMPLETE:
         pboottable->tag = TAG_UPDATE_ETC;
         break;

      default:
         install_test("Error! should not happened TAG:%d\r\n", pboottable->tag);
   }

   return _eRTK_SUCCESS;
}
#endif //EMMC_SUPPORT

#if defined(NAS_ENABLE) && defined(CONFIG_BOOT_FROM_SPI)
int tagflow3_spi(S_BOOTTABLE* pboottable, struct t_rtkimgdesc* prtkimgdesc, S_BOOTTABLE* pbt)
{
    int ret = 0;

    switch(pboottable->tag) {
        case TAG_UNKNOWN:
            pboottable->tag = TAG_UPDATE_ETC;
            break;

        case TAG_UPDATE_ETC:
            if (prtkimgdesc->fw[FW_USR_LOCAL_ETC].img_size == 0) {
                install_info("There is no overlay partition\n");
            }

            if (prtkimgdesc->update_etc == 1 && prtkimgdesc->mode != _UPGRAD) {
                install_log("update_etc=y, start update etc\r\n");
                if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
                    remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
                    add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC]);
                }
                else {
                    install_fail("Burn overlay fail\r\n");
                    return -_eRTK_TAG_UPDATE_ETC_FAIL;
                }
            }
            /* TODO: etc partition existance, size and offset check? */
            pboottable->tag = TAG_UPDATE_NL;
            break;

        case TAG_UPDATE_NL:
            if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL])) == 0) {
                update_fw(pboottable, FWTYPE_KERNEL, &prtkimgdesc->fw[FW_KERNEL]);
            }
            else {
                install_fail("Burn KERNEL fail\r\n");
                return -_eRTK_TAG_UPDATE_NL_FAIL;
            }
            pboottable->tag = TAG_UPDATE_RL_DT;
            break;

        case TAG_UPDATE_RL_DT:
            if( prtkimgdesc->install_dtb == 1 ) {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_DT])) == 0) {
                    update_fw(pboottable, FWTYPE_RESCUE_DT, &prtkimgdesc->fw[FW_RESCUE_DT]);
                }
                else {
                    install_fail("Burn RSCUE_DT fail\r\n");
                    return -_eRTK_TAG_UPDATE_RL_DT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_NL_DT;
            break;

        case TAG_UPDATE_NL_DT:
            if( prtkimgdesc->install_dtb == 1 ) {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_DT])) == 0) {
                    update_fw(pboottable, FWTYPE_KERNEL_DT, &prtkimgdesc->fw[FW_KERNEL_DT]);
                }
                else {
                    install_fail("Burn KERNEL_DT fail\r\n");
                    return -_eRTK_TAG_UPDATE_NL_DT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_RL_ROOTFS;
            break;

        case TAG_UPDATE_RL_ROOTFS:
            if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_ROOTFS])) == 0) {
                update_fw(pboottable, FWTYPE_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_RESCUE_ROOTFS]);
            }
            else {
                install_fail("Burn RESCUE_ROOTFS fail\r\n");
                return -_eRTK_TAG_UPDATE_RL_ROOTFS_FAIL;
            }
            pboottable->tag = TAG_UPDATE_AK;
            break;

        case TAG_UPDATE_AK:
            if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_AUDIO])) == 0) {
                update_fw(pboottable, FWTYPE_AKERNEL, &prtkimgdesc->fw[FW_AUDIO]);
            }
            else {
                install_fail("Burn AUDIO FW fail\r\n");
                return -_eRTK_TAG_UPDATE_AK_FAIL;
            }
            pboottable->tag = TAG_UPDATE_ROOTFS;
            break;

      case TAG_UPDATE_ROOTFS:
		 if (prtkimgdesc->fw[FW_ROOTFS].img_size == 0) {
			install_info("There is no ROOTFS partition\n");
		 }
		 else {
         	if ((ret = rtk_burn_single_part(prtkimgdesc, FW_ROOTFS)) == 0) {
            	remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_ROOTFS].part_name);
            	add_part(pboottable, &prtkimgdesc->fw[FW_ROOTFS]);
         	}
         	else {
            	install_fail("Burn ROOTFS fail\r\n");
         	}
		 }
         pboottable->tag = TAG_COMPLETE;
         break;

      case TAG_COMPLETE:
         pboottable->tag = TAG_UPDATE_ETC;
         break;

      default:
         install_test("Error! should not happened TAG:%d\r\n", pboottable->tag);
   }

   return _eRTK_SUCCESS;
}
#endif //NOR flash

int tagflow3_sata(S_BOOTTABLE* pboottable, struct t_rtkimgdesc* prtkimgdesc, S_BOOTTABLE* pbt)
{
    int ret = 0;
    enum FWTYPE efwtype;
    //printf("[Installer_D]: 5.SATA prtkimg->mtdblock_path = [%s].\n", prtkimgdesc->mtdblock_path);
    
    switch(pboottable->tag) {
        case TAG_UNKNOWN:
            pboottable->tag = TAG_UPDATE_ETC;
        break;
    
        case TAG_UPDATE_ETC:
            if (prtkimgdesc->fw[FW_USR_LOCAL_ETC].img_size == 0) {
                install_info("There is no USR_LOCAL_ETC partition\n");
            }
            else {
                if((ret = rtk_burn_single_part(prtkimgdesc, FW_USR_LOCAL_ETC)) == 0) {
                    remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_USR_LOCAL_ETC].part_name);
                    add_part(pboottable, &prtkimgdesc->fw[FW_USR_LOCAL_ETC]);
                }
                else {
                    install_fail("Burn ETC fail\r\n");
                }
            }

            pboottable->tag = TAG_UPDATE_GOLD_NL;
            break;
            
        case TAG_UPDATE_GOLD_NL:
            install_info("TAG_UPDATE_GOLD_NL.\n");
            if (prtkimgdesc->fw[FW_GOLD_KERNEL].img_size != 0){
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_KERNEL])) == 0) {
                    update_fw(pboottable, FWTYPE_GOLD_KERNEL, &prtkimgdesc->fw[FW_GOLD_KERNEL]);
                }
                else {
                    install_fail("Burn FW_GOLD_KERNEL fail\r\n");
                    return -_eRTK_TAG_UPDATE_G_NL_FAIL;
                }
            }
            else
                install_info("TAG_UPDATE_GOLD_NL size = %d.\n", prtkimgdesc->fw[FW_GOLD_KERNEL].img_size);
            pboottable->tag = TAG_UPDATE_GOLD_RL_DT;
            break;
    
        case TAG_UPDATE_GOLD_RL_DT:
            install_info("TAG_UPDATE_GOLD_RL_DT.\n");
            if (prtkimgdesc->fw[FW_GOLD_RESCUE_DT].img_size != 0){
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_RESCUE_DT])) == 0) {
                    update_fw(pboottable, FWTYPE_GOLD_RESCUE_DT, &prtkimgdesc->fw[FW_GOLD_RESCUE_DT]);
                }
                else {
                    install_fail("Burn FW_GOLD_RESCUE_DT fail\r\n");
                    return -_eRTK_TAG_UPDATE_G_RL_DT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_GOLD_RL_ROOTFS;
            break;
    
        case TAG_UPDATE_GOLD_RL_ROOTFS:
            if (prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS].img_size != 0){
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS])) == 0) {
                    update_fw(pboottable, FWTYPE_GOLD_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_GOLD_RESCUE_ROOTFS]);
                }
                else {
                    install_fail("Burn FW_GOLD_RESCUE_ROOTFS fail\r\n");
                    return -_eRTK_TAG_UPDATE_G_RL_ROOTFS_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_GOLD_AK;
            break;
             
        case TAG_UPDATE_GOLD_AK:
            if (prtkimgdesc->fw[FW_GOLD_AUDIO].img_size != 0){
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_GOLD_AUDIO])) == 0) {
                    update_fw(pboottable, FWTYPE_GOLD_AKERNEL, &prtkimgdesc->fw[FW_GOLD_AUDIO]);
                }
                else {
                    install_fail("Burn FW_GOLD_AUDIO fail\r\n");
                    return -_eRTK_TAG_UPDATE_G_AK_FAIL;
                }
            } 
            pboottable->tag = TAG_UPDATE_NL;
            break;

        case TAG_UPDATE_NL:
            if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL])) == 0) {
                update_fw(pboottable, FWTYPE_KERNEL, &prtkimgdesc->fw[FW_KERNEL]);
            }
            else {
                install_fail("Burn KERNEL fail\r\n");
                return -_eRTK_TAG_UPDATE_NL_FAIL;
            }
            pboottable->tag = TAG_UPDATE_RL_DT;
            break;
    
        case TAG_UPDATE_RL_DT:
            if( prtkimgdesc->install_dtb == 1 ) {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_DT])) == 0) {
                    update_fw(pboottable, FWTYPE_RESCUE_DT, &prtkimgdesc->fw[FW_RESCUE_DT]);
                }
                else {
                    install_fail("Burn RSCUE_DT fail\r\n");
                    return -_eRTK_TAG_UPDATE_RL_DT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_NL_DT;
            break;
    
        case TAG_UPDATE_NL_DT:
            if( prtkimgdesc->install_dtb == 1 ) {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_DT])) == 0) {
                    update_fw(pboottable, FWTYPE_KERNEL_DT, &prtkimgdesc->fw[FW_KERNEL_DT]);
                }
                else {
                    install_fail("Burn KERNEL_DT fail\r\n");
                    return -_eRTK_TAG_UPDATE_NL_DT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_RL_ROOTFS;
            break;
    
        case TAG_UPDATE_RL_ROOTFS:
            if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_RESCUE_ROOTFS])) == 0) {
                update_fw(pboottable, FWTYPE_RESCUE_ROOTFS, &prtkimgdesc->fw[FW_RESCUE_ROOTFS]);
            }
            else {
                install_fail("Burn RESCUE_ROOTFS fail\r\n");
                return -_eRTK_TAG_UPDATE_RL_ROOTFS_FAIL;
            }

            pboottable->tag = TAG_UPDATE_NL_ROOTFS;
            break;
        
        case TAG_UPDATE_NL_ROOTFS:
#if defined(NAS_ENABLE) || defined(GENERIC_LINUX)
            if (prtkimgdesc->fw[FW_KERNEL_ROOTFS].img_size == 0) {
                install_info("There is no KERNEL_ROOTFS partition\n");
            }
            else
#endif
            {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_KERNEL_ROOTFS])) == 0) {
                    update_fw(pboottable, FWTYPE_KERNEL_ROOTFS, &prtkimgdesc->fw[FW_KERNEL_ROOTFS]);
            }
                else {
                    install_fail("Burn KERNEL_ROOTFS fail\r\n");
                    return -_eRTK_TAG_UPDATE_NL_ROOTFS_FAIL;
                }
            }
             pboottable->tag = TAG_UPDATE_TEE;
             break;
    
        case TAG_UPDATE_TEE:
#ifdef TEE_ENABLE
            if (prtkimgdesc->fw[FW_TEE].img_size == 0) {
                install_info("There is no TEE FW\n");
            }
            else
            {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_TEE])) == 0) {
                    update_fw(pboottable, FWTYPE_TEEKERNEL, &prtkimgdesc->fw[FW_TEE]);
                }
                else {
                    install_fail("Burn TEE FW fail\r\n");
                    return -_eRTK_TAG_UPDATE_TEEK_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_BL31;
            break;
    
        case TAG_UPDATE_BL31:
            if (prtkimgdesc->fw[FW_BL31].img_size == 0) {
                install_info("There is no BL31 FW\n");
            }
            else
            {
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_BL31])) == 0) {
                    update_fw(pboottable, FWTYPE_BL31, &prtkimgdesc->fw[FW_BL31]);
                }
                else {
                    install_fail("Burn BL31 FW fail\r\n");
                    return -_eRTK_TAG_UPDATE_BL31_FAIL;
                }
            }
#endif
             pboottable->tag = TAG_UPDATE_XEN;
             break;
    
        case TAG_UPDATE_XEN: 
#ifdef NAS_ENABLE
#ifdef HYPERVISOR
             if (prtkimgdesc->fw[FW_XEN].img_size == 0)
             {
                install_info("There is no XEN_OS image.\n");
             }
             else
             {
                 if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_XEN])) == 0) {
                    update_fw(pboottable, FWTYPE_XEN, &prtkimgdesc->fw[FW_XEN]);
                 }
                 else {
                    install_fail("Burn XEN_OS fail\r\n");
                    return -_eRTK_TAG_UPDATE_XEN_FAIL;
                 }
             }
#endif
#endif
             pboottable->tag = TAG_UPDATE_AK;
             break;
             
        case TAG_UPDATE_AK:
#ifdef NAS_ENABLE
             if (prtkimgdesc->fw[FW_AUDIO].img_size == 0) {
                install_info("There is no AUDIO FW\n");
             }
             else
#endif
             {
             if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_AUDIO])) == 0) {
                update_fw(pboottable, FWTYPE_AKERNEL, &prtkimgdesc->fw[FW_AUDIO]);
             }
             else {
                install_fail("Burn AUDIO FW fail\r\n");
                return -_eRTK_TAG_UPDATE_AK_FAIL;
             }
             }
             pboottable->tag = TAG_UPDATE_DALOGO;
             //pboottable->tag = TAG_COMPLETE;
             //pboottable->boottype = BOOTTYPE_COMPLETE;
             break;
    
        case TAG_UPDATE_DALOGO:
            for( unsigned int i=0; i<SIZEOF_ARRAY(audioFW); i++ )
            {
                if ( prtkimgdesc->fw[ audioFW[i].fwtype ].img_size == 0 ) {
                    install_info("There are no Audio bootfile(%d)\n", i);
                    update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
                    continue;
                }
    
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ audioFW[i].fwtype ])) == 0) {
                    update_fw(pboottable, audioFW[i].e_type, &prtkimgdesc->fw[ audioFW[i].fwtype ]);
                }
                else {
                    install_debug("Burn AUDIO LOGO(%d) fail\nSet DALOGO target, offset & size to 0\n", i);
                    update_fw(pboottable, audioFW[i].e_type, 0,0,0,NULL);
                }
            }
            pboottable->tag = TAG_UPDATE_DILOGO;
            break;
    
        case TAG_UPDATE_DILOGO:
            for( unsigned int i=0; i<SIZEOF_ARRAY(imageFW); i++ )
            {
                if ( prtkimgdesc->fw[ imageFW[i].fwtype ].img_size == 0 ) {
                    install_info("There are no Image bootfile(%d)\n", i);
                    update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
                    continue;
                }

                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ imageFW[i].fwtype ])) == 0) {
                    update_fw(pboottable, imageFW[i].e_type, &prtkimgdesc->fw[ imageFW[i].fwtype ]);
                }
                else {
                    install_debug("Burn IMAGE LOGO(%d) fail\nSet DILOGO target, offset & size to 0\n",i);
                    update_fw(pboottable, imageFW[i].e_type, 0,0,0,NULL);
                }
            }
            pboottable->tag = TAG_UPDATE_DVLOGO;
            break;
    
        case TAG_UPDATE_DVLOGO:
            for( unsigned int i=0; i<SIZEOF_ARRAY(videoFW); i++ )
            {
                if ( prtkimgdesc->fw[ videoFW[i].fwtype ].img_size == 0 ) {
                    install_info("There are no Video bootfile(%d)\n", i);
                    update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
                    continue;
                }
    
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[ videoFW[i].fwtype ])) == 0) {
                    update_fw(pboottable, videoFW[i].e_type, &prtkimgdesc->fw[ videoFW[i].fwtype ]);
                }
                else {
                    install_debug("Burn VIDEO LOGO(%d) fail\nSet DVLOGO target, offset & size to 0\n",i);
                    update_fw(pboottable, videoFW[i].e_type, 0,0,0,NULL);
                }
            }
            pboottable->tag = TAG_UPDATE_ROOTFS;
            break;
    
        case TAG_UPDATE_ROOTFS:
            if (prtkimgdesc->fw[FW_ROOTFS].img_size == 0) {
                install_info("There is no ROOTFS partition\n");
            }
            else {
                if ((ret = rtk_burn_single_part(prtkimgdesc, FW_ROOTFS)) == 0) {
                    remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_ROOTFS].part_name);
                    add_part(pboottable, &prtkimgdesc->fw[FW_ROOTFS]);
                }
                else {
                    install_fail("Burn ROOTFS fail\r\n");
                }
            }
            pboottable->tag = TAG_UPDATE_RES;
            break;
    
        case TAG_UPDATE_RES:
            if (prtkimgdesc->fw[FW_RES].img_size == 0) {
                install_info("There is no RES partition\n");
                pboottable->tag = TAG_UPDATE_CACHE;
                break;
            }
    
            if ((ret = rtk_burn_single_part(prtkimgdesc, FW_RES)) == 0) {
                remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_RES].part_name);
                add_part(pboottable, &prtkimgdesc->fw[FW_RES]);
            }
            else {
                install_fail("Burn RES fail\r\n");
                return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
            }
            pboottable->tag = TAG_UPDATE_CACHE;
            break;
    
        case TAG_UPDATE_CACHE:
#ifdef NAS_ENABLE
            if (prtkimgdesc->fw[FW_CACHE].img_size == 0) {
                install_info("There is no CACHE partition\n");
                pboottable->tag = TAG_UPDATE_DATA;
                break;
            }
#endif
            if ((ret = rtk_burn_single_part(prtkimgdesc, FW_CACHE)) == 0) {
                remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_CACHE].part_name);
                add_part(pboottable, &prtkimgdesc->fw[FW_CACHE]);
            }
            else {
                install_fail("Burn CACHE fail\r\n");
                return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
            }
            pboottable->tag = TAG_UPDATE_DATA;
            break;
    
        case TAG_UPDATE_DATA:
#ifdef NAS_ENABLE
            if (prtkimgdesc->fw[FW_DATA].img_size == 0) {
                install_info("There is no DATA partition\n");
                pboottable->tag = TAG_UPDATE_SYSTEM;
                break;
            }
#endif
             if ((ret = rtk_burn_single_part(prtkimgdesc, FW_DATA)) == 0) {
                remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_DATA].part_name);
                add_part(pboottable, &prtkimgdesc->fw[FW_DATA]);
             }
             else {
                install_fail("Burn DATA fail\r\n");
                return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
             }
             pboottable->tag = TAG_UPDATE_SYSTEM;
             break;
    
        case TAG_UPDATE_SYSTEM:
#ifdef NAS_ENABLE
            if (prtkimgdesc->fw[FW_SYSTEM].img_size == 0) {
                install_info("There is no SYSTEM partition\n");
                pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
                break;
            }
#endif
            if ((ret = rtk_burn_single_part(prtkimgdesc, FW_SYSTEM)) == 0) {
                remove_part_by_partname(pboottable, prtkimgdesc->fw[FW_SYSTEM].part_name);
                add_part(pboottable, &prtkimgdesc->fw[FW_SYSTEM]);
            }
            else {
                install_fail("Burn SYSTEM fail\r\n");
                return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
            }
#ifndef NFLASH_LAOUT
            pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
            break;
#else
            pboottable->tag = TAG_UPDATE_UBOOT;
            break;
             
        case TAG_UPDATE_UBOOT:
            if (prtkimgdesc->fw[FW_UBOOT].img_size != 0){
                if ((ret = rtk_burn_img(prtkimgdesc, &prtkimgdesc->fw[FW_UBOOT])) == 0) {
                    update_fw(pboottable, FWTYPE_P_UBOOT, &prtkimgdesc->fw[FW_UBOOT]);
                }
                else {
                    install_fail("Burn FW_UBOOT fail\r\n");
                    return -_eRTK_TAG_UPDATE_UBOOT_FAIL;
                }
            }
            pboottable->tag = TAG_UPDATE_OTHER_PARTITION1;
            break;
#endif
        case TAG_UPDATE_OTHER_PARTITION1:
            for (efwtype = P_PARTITION1; efwtype <= P_PARTITION8; efwtype=FWTYPE(efwtype+1)) {
                if (prtkimgdesc->fw[efwtype].flash_allo_size != 0 && prtkimgdesc->fw[efwtype].img_size!=0 && strlen(prtkimgdesc->fw[efwtype].part_name) != 0) {
 #ifndef PC_SIMULATE   
                    if (gDebugPrintfLogLevel&INSTALL_MEM_LEVEL)
                        system("cat /proc/meminfo");
 #endif   
                    if ((strcmp(prtkimgdesc->fw[efwtype].part_name, "uboot") == 0) 
                        || (strcmp(prtkimgdesc->fw[efwtype].part_name, "logo") == 0)){
#ifdef NAS_ENABLE
                        remove_part_by_partname(pboottable, prtkimgdesc->fw[efwtype].part_name);
#endif
                        add_part(pboottable, &prtkimgdesc->fw[efwtype]);
                    }
                    else{
                        if ((ret = rtk_burn_single_part(prtkimgdesc, efwtype)) == 0) {
#ifdef NAS_ENABLE
                            remove_part_by_partname(pboottable, prtkimgdesc->fw[efwtype].part_name);
#endif
                            add_part(pboottable, &prtkimgdesc->fw[efwtype]);
                        } else {
                            install_fail("Burn Partition(%s) fail\r\n", prtkimgdesc->fw[efwtype].part_name );
                            return -_eRTK_TAG_UPDATE_PARTITION_FAIL;
                        }
                    }
                }
            }
    
            //update SSUWORKPART
            update_ssu_work_part(pboottable, prtkimgdesc->next_ssu_work_part);
    
            pboottable->tag = TAG_COMPLETE;
            pboottable->boottype = BOOTTYPE_COMPLETE;
            break;
    
        case TAG_COMPLETE:
            pboottable->tag = TAG_UPDATE_ETC;
            break;
    
        default:
            install_test("Error! should not happened TAG:%d\r\n", pboottable->tag);
    }
    
    return _eRTK_SUCCESS;
}

