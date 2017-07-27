#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <rtk_common.h>
#include <rtk_tar.h>
#include <rtk_urltar.h>
#include <rtk_imgdesc.h>
#include <rtk_fwdesc.h>
#include <rtk_mtd.h>

int rtk_open_tarfile_with_offset(struct tarinfo* ptarinfo, unsigned int tarfile_offset)
{
   int ret;
   if(ptarinfo->fd < 0)
   {
      ret = open(ptarinfo->tarfile_path, O_RDONLY);
      if(ret < 0)
      {
         install_debug("Can't open %s\r\n", ptarinfo->tarfile_path);
         return -1;
      }
      ptarinfo->fd = ret;
   }

   ret = lseek(ptarinfo->fd, tarfile_offset, SEEK_SET);
   if(ret < 0)
   {
      install_debug("Can't lseek %s (%lu)\r\n", ptarinfo->tarfile_path, ptarinfo->tarfile_offset);
      return -1;
   }
   return 0;
}

//#define TAR_DEBUG
static int rtk_dump_hash_value(struct tarinfo* ptarinfo, unsigned int tarfile_offset)
{
   int i;
   int ret;
   char hash_value[128];
   ret = rtk_open_tarfile_with_offset(ptarinfo, tarfile_offset);
   if(ret < 0)
   {
      install_debug("Can't open tarfile\r\n");
      return -1;
   }
   ret = read(ptarinfo->fd, hash_value, sizeof(hash_value));
   close(ptarinfo->fd);
   ptarinfo->fd = -1;
#ifdef TAR_DEBUG
   install_debug("hash_value : \r\n");
   for(i=0;i<ret;i++)
   {
      install_debug("0x%02X ", (unsigned short int)hash_value[i]);
   }
   install_debug("\r\n");
#endif   
   return 0;
}

static int rtk_get_tarhead_from_tarfile(struct tarinfo* ptarinfo, struct t_tarheader* pthead)
{
   int ret;
   if(ptarinfo->fd < 0)
   {
      ret = open(ptarinfo->tarfile_path, O_RDONLY);
      if(ret < 0)
      {
         install_debug("Can't open %s\r\n", ptarinfo->tarfile_path);
         return -1;
      }
      ptarinfo->fd = ret;
   }
   ret = lseek(ptarinfo->fd, ptarinfo->tarfile_offset, SEEK_SET);
   if(ret < 0)
   {
      install_debug("Can't lseek %s (%lu)\r\n", ptarinfo->tarfile_path, ptarinfo->tarfile_offset);
      return -1;
   }
   ret = read(ptarinfo->fd, (void*) pthead, sizeof(struct t_tarheader));
   if(ret == 0)
   {
      install_debug("read EOF\r\n");
      return 0;
   }
   else if(ret < -1)
   {
      install_debug("read error\r\n");
      return -1;
   }
   else if(ret!=sizeof(struct t_tarheader))
   {
      install_debug("read fail, ret=%d\r\n", ret);
      return -1;
   }
   return ret;
}

static struct t_tarheader tmpthead[50];
static unsigned int tmpcount;
// function parse_urltar
// purl:IN
// prtkimg:OUT
int parse_tar(struct t_rtkimgdesc* prtkimg)
{
   struct t_tarheader thead;
   int ret;
   unsigned int size;

   // sanity-check
   //install_log("NULL:%d ZERO:%d, prtkimg->tarinfo.tarfile_path=%s(0x%08x)\r\n"
   //            , (prtkimg->tarinfo.tarfile_path == NULL)
   //            , (strlen(prtkimg->tarinfo.tarfile_path)==0)
   //            , prtkimg->tarinfo.tarfile_path, prtkimg->tarinfo.tarfile_path);
   
   //if( (prtkimg->tarinfo.tarfile_path == NULL) || (strlen(prtkimg->tarinfo.tarfile_path)==0) )
   install_info("@parse_tar       %s   %d\n",prtkimg->tarinfo.tarfile_path,strlen(prtkimg->tarinfo.tarfile_path) );
   
   if(strlen(prtkimg->tarinfo.tarfile_path) == 0)
   {
      install_debug("rtkimg.tarinfo.tarfile_path is NULL\r\n");
      return -1;
   }

   // fill tarinfo
   ret = rtk_get_size_of_file(prtkimg->tarinfo.tarfile_path, &size);
   if(ret < 0)
   {
      install_debug("rtk_get_size_of_tarfile fail\r\n");
      return -1;
   }
   prtkimg->tarinfo.fd = -1;
   prtkimg->tarinfo.tarfile_offset = 0;
   prtkimg->tarinfo.tarfile_size = size;

   install_info("tarfile_path:%s, tarfile_size: %u Bytes (%u KB)\r\n" \
         , prtkimg->tarinfo.tarfile_path, prtkimg->tarinfo.tarfile_size, prtkimg->tarinfo.tarfile_size/1024);

   rtk_dump_hash_value(&prtkimg->tarinfo, prtkimg->tarinfo.tarfile_size-16);
   tmpcount = 0;
   
   while(prtkimg->tarinfo.tarfile_offset < prtkimg->tarinfo.tarfile_size)
   {
      //printf("current tarfile_offset:%u\r\n" , prtkimg->tarinfo.tarfile_offset);
      ret = rtk_get_tarhead_from_tarfile(&prtkimg->tarinfo, &thead);
      if(ret < 0)
      {
         install_debug("rtk_get_tarhead_from_tarfile() error\r\n");
         break;
      }
      else if(0 == ret)
      {
         install_debug("rtk_get_tarhead_from_tarfile() close\r\n");
         break;
      }
      else
      {
         // check tag header's magic
         //if(strncmp(thead.ustarin, "ustar ", 6)) break;
         if(thead.ustarin[0] == 0 || strncmp(thead.ustarin, "ustar", 5))
         {
            install_debug("tar magic error\r\n");
            break;
         }
         if(!strlen(thead.filename))
         {
            install_debug("filename is NULL\r\n");
            break;
         }

         // check tag header's checksum
         // TODO: checksum is not implemented

         if(tmpcount < ARRAY_COUNT(tmpthead))
         {
            thead.last.tarfile_offset = prtkimg->tarinfo.tarfile_offset;
            memcpy((void*) &tmpthead[tmpcount], (void*) &thead, sizeof(struct t_tarheader));
            tmpcount++;
         }

         install_debug("filename = %-60s\nfilesize = %9u (%5uKB), ustarin = %s\r\n"
               , thead.filename
               , octalStringToInt(thead.filesize, FILESIZELEN)
               , octalStringToInt(thead.filesize, FILESIZELEN)/1024
               , thead.ustarin);
         //printf("Tarfile parsing offset:%d (0x%08x)\r\n", prtkimg->tarinfo.tarfile_offset, prtkimg->tarinfo.tarfile_offset);

         fill_rtkimgdesc_file(&thead, prtkimg->tarinfo.tarfile_offset, prtkimg);

         // the next tar header offset
         prtkimg->tarinfo.tarfile_offset = prtkimg->tarinfo.tarfile_offset \
                                          + 512 \
                                          + ((octalStringToInt(thead.filesize, FILESIZELEN)-1)/512+1)*512;
      }
   }
   prtkimg->thead_count = tmpcount;
   prtkimg->thead_list = tmpthead;
   // TODO: compatible for configuration xml
   // to get etc_usr_local partition size
// if(prtkimg->config.img_size)
//    implement_configuration_compatible(prtkimg, (void*) &tmpthead[0], tmpcount);
   if(!tmpcount)
   {
      install_fail("empty install.img\r\n");
      close(prtkimg->tarinfo.fd);
      prtkimg->tarinfo.fd = -1;
      return -1;
   }
   close(prtkimg->tarinfo.fd);
   prtkimg->tarinfo.fd = -1;
   prtkimg->tarfileparsed = 1;
   return 0;
}

// function parse_urltar
// purl:IN
// prtkimg:OUT
// function parse_urltar
// purl:IN
// prtkimg:OUT
int parse_urltar(struct t_rtkimgdesc* prtkimg)
{
   struct t_tarheader thead;
   int result, ret;
   int sock;

   ret = rtk_get_size_of_tarfile_from_urltar(&prtkimg->url);
   if(ret < 0) return -1;

   install_info("URL:%s\r\n"
         "The filesize of tarfile: %u\r\n", prtkimg->url.url, prtkimg->url.tarfile_size);
   prtkimg->url.tarfile_offset = 0;
   sock = -1;
   tmpcount = 0;
   
   while(prtkimg->url.tarfile_offset < prtkimg->url.tarfile_size)
   {
      install_debug("\r\nTarfile parsing offset:%d (0x%08x)\r\n", prtkimg->url.tarfile_offset, prtkimg->url.tarfile_offset);
      result = rtk_get_tarhead_from_urltar(&sock, &prtkimg->url, &thead);
      if(result < 0)
      {
         install_debug("rtk_get_tarhead_from_urltar() fail\r\n");
         sock = -1;
         continue;

         close(sock);
         return -1;
      }
      else if(0 == result)
      {
         install_debug("rtk_get_tarhead_from_urltar() connection close, restart\r\n");
         sock = -1;
         continue;
      }
      else
      {
         // check tag header's magic
         //if(strncmp(thead.ustarin, "ustar ", 6)) break;
         if(!strlen(thead.filename))
         {
            install_debug("filename=%s, filezie=%u, ustarin=%s\r\n"
               , thead.filename
               , octalStringToInt(thead.filesize, FILESIZELEN)
               , thead.ustarin);
            break;
         }
         // check tag header's checksum
         // TODO: checksum is not implemented

         if(tmpcount < ARRAY_COUNT(tmpthead))
         {
            thead.last.tarfile_offset = prtkimg->url.tarfile_offset;
            memcpy((void*) &tmpthead[tmpcount], (void*) &thead, sizeof(struct t_tarheader));
            tmpcount++;
         }

         install_debug("filename=%s, filezie=%u, ustarin=%s\r\n"
               , thead.filename
               , octalStringToInt(thead.filesize, FILESIZELEN)
               , thead.ustarin);
         fill_rtkimgdesc_file(&thead, prtkimg->url.tarfile_offset, prtkimg);

         // the next tar header offset
         prtkimg->url.tarfile_offset = prtkimg->url.tarfile_offset \
                                    + 512 \
                                    + ((octalStringToInt(thead.filesize, FILESIZELEN)-1)/512+1)*512;
      }
   }
   prtkimg->thead_count = tmpcount;
   prtkimg->thead_list = tmpthead;
   // TODO: compatible for configuration xml
   // to get etc_usr_local partition size
// if(prtkimg->config.img_size)
//    implement_configuration_compatible(prtkimg, (void*) &tmpthead[0], tmpcount);

   close(sock);
   return 0;
}

int parse_flashtar(unsigned int backup_start_offset, unsigned int backup_end_offset, struct t_rtkimgdesc* prtkimg)
{
   struct t_tarheader thead;
   int ret;
   char* mtdblock_path;

   // open mtdblock
   ret = rtk_open_mtd_block(&mtdblock_path);
   if(ret < 0)
   {
      install_debug("rtk_open_mtd_block fail\r\n");
      return -1;
   }

   // fill tarinfo
   sprintf(prtkimg->tarinfo.tarfile_path, "%s", mtdblock_path);
   prtkimg->tarinfo.fd = ret;
   prtkimg->tarinfo.tarfile_offset = backup_start_offset;
   prtkimg->tarinfo.tarfile_size = backup_end_offset;

   install_log("tarfile_path:%s, tarfile_start=0x%08x, tarfile_end=0x%08x\r\n" \
         , prtkimg->tarinfo.tarfile_path, prtkimg->tarinfo.tarfile_offset, prtkimg->tarinfo.tarfile_size);
   tmpcount = 0;
   
   while(prtkimg->tarinfo.tarfile_offset < prtkimg->tarinfo.tarfile_size)
   {
      ret = rtk_get_tarhead_from_tarfile(&prtkimg->tarinfo, &thead);
      if(0 == ret)
      {
         install_log("rtk_get_tarhead_from_tarfile() close\r\n");
         break;
      }
      else
      {
         // check tag header's magic
         if(thead.ustarin[0] == 0 || strncmp(thead.ustarin, "ustar", 5))
         {
            install_debug("tar magic error\r\n");
            install_debug("filename=%s, filezie=%u Bytes (%uKB), ustarin=%s\r\n"
                     , thead.filename
                     , octalStringToInt(thead.filesize, FILESIZELEN)
                     , octalStringToInt(thead.filesize, FILESIZELEN)/1024
                     , thead.ustarin);
            break;
         }
         if(!strlen(thead.filename))
         {
            install_debug("filename is NULL\r\n");
            break;
         }
         // check tag header's checksum
         // TODO: checksum is not implemented

         // record
         if(tmpcount< ARRAY_COUNT(tmpthead))
         {
            thead.last.tarfile_offset = prtkimg->tarinfo.tarfile_offset;
            memcpy((void*) &tmpthead[tmpcount], (void*) &thead, sizeof(struct t_tarheader));
            tmpcount++;
         }

         install_debug("filename=%s, filezie=%u Bytes (%uKB), ustarin=%s\r\n"
               , thead.filename
               , octalStringToInt(thead.filesize, FILESIZELEN)
               , octalStringToInt(thead.filesize, FILESIZELEN)/1024
               , thead.ustarin);
         //printf("Tarfile parsing offset:%d (0x%08x)\r\n", prtkimg->tarinfo.tarfile_offset, prtkimg->tarinfo.tarfile_offset);

         fill_rtkimgdesc_file(&thead, prtkimg->tarinfo.tarfile_offset, prtkimg);

         //the next tar header offset
         prtkimg->tarinfo.tarfile_offset = prtkimg->tarinfo.tarfile_offset \
                                          + 512 \
                                          + ((octalStringToInt(thead.filesize, FILESIZELEN)-1)/512+1)*512;
      }
   }
   prtkimg->thead_count = tmpcount;
   prtkimg->thead_list = tmpthead;
   // TODO: compatible for configuration xml
   // to get etc_usr_local partition size
   // if(prtkimg->config.img_size)
   //    implement_configuration_compatible(prtkimg, (void*) &tmpthead[0], tmpcount);
   if(!tmpcount)
   {
      install_debug("empty install.img\r\n");
      close(prtkimg->tarinfo.fd);
      return -1;
   }
   close(prtkimg->tarinfo.fd);
   prtkimg->tarinfo.fd = -1;
   prtkimg->tarfileparsed = 1;
   return 0;
}
#if 0
// for loader_a
unsigned int get_install_offset_from_flash(unsigned int startAddress)
{
   int ret;
   struct t_rtkimgdesc rtkimgdesc;

   // initial rtkimgdesc's meminfo
   ret = fill_rtkimgdesc_meminfo(&rtkimgdesc);
   if(ret < 0)
   {
      install_debug("fill_rtkimgdesc_meminfo fail\r\n");
      return 0;
   }

   // parsing flashtar
   ret = parse_flashtar(startAddress, 0xffffffff, &rtkimgdesc);
   if(ret < 0)
   {
      install_debug("parse_tar fail\r\n");
      return 0;
   }

   // fill rtkimgdesc's offset
   fill_rtkimgdesc_layout(&rtkimgdesc);

   install_log("install_a's offset :0x%08x\r\n", rtkimgdesc.install_a.tarfile_offset);
   if(rtkimgdesc.install_a.img_size== 0)
   {
      install_debug("Can't find install_a\r\n");
      return 0;
   }
   return rtkimgdesc.install_a.tarfile_offset-startAddress;
}

#endif
