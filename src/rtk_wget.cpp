#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <rtk_common.h>
#include <rtk_tar.h>
#include <rtk_imgdesc.h>
#include <rtk_mtd.h>
#include <rtk_wget.h>
int rtk_wget(const char* purlstr
, const char* filename
, const char* stored_filename)
{
#define BUF_MAXSIZE 1024*8

	static struct t_rtkimgdesc rtkimg;
	static char urlstr[128]="";

	int ret, i;
	struct t_tarheader* pthead;
	int fd, sock;
	struct t_imgdesc img;
	struct t_imgdesc* pimgdesc;
	pimgdesc = &img;


	// memory prev urlstr
	if(strcmp(urlstr, purlstr) != 0)
	{
		sprintf(urlstr, "%s", purlstr);
		// parsing the url
		ret = urlparse(purlstr, &rtkimg.url);
		if(ret < 0)
		{
			install_debug("urlparse() fail\r\n");
			return -1;
		}
		// parsing urltar
		ret = parse_urltar(&rtkimg);
		if(ret < 0)
		{
			install_debug("parse_urltar() fail\r\n");
			return ret;
		}
	}
	// search the filename
	pthead = NULL;
	for(i=0;i<rtkimg.thead_count;i++)
	{
		if((strlen(rtkimg.thead_list[i].filename) == strlen(filename)) && \
			(0 == strcmp(rtkimg.thead_list[i].filename, filename)) )
		{
			pthead = &rtkimg.thead_list[i];
			printf("Got %s, thead_list(%d)\r\n", filename, i);
		}
	}
	// check if the filename exist
	if(NULL == pthead)
	{
		install_debug("Can't get the filename:%s\r\n", filename);
		return -1;
	}

	pimgdesc->tarfile_offset = pthead->last.tarfile_offset;
	pimgdesc->img_size = octalStringToInt(pthead->filesize, 11);
	pimgdesc->install_offset = 0;

	// set the stored_filename
	if(NULL == stored_filename)
		stored_filename = filename;
	// open the stored_filename
	fd = open(stored_filename, O_CREAT|O_WRONLY|O_TRUNC);
	if(fd < 0)
	{
		install_debug("Can't create file:%s\r\n", stored_filename);
		return -1;
	}
	// open network streaming
	ret = rtk_open_tarfile_from_urltar(&sock, &rtkimg.url, pimgdesc->tarfile_offset, pimgdesc->install_offset, pimgdesc->img_size);
	if(0 >= ret)
	{
		close(fd);
		install_debug("rtk_open_tarfile_from_urltar() fail, ret=%d\r\n", ret);
		return -1;
	}

	// Downloading
	// read data from network
	// write data into file
	printf("\r\nDownloading %s\r\n", filename);
	ret = fd_to_fd(sock, fd, pimgdesc->img_size);
#if 0
	int rret, wret;
	char data_buf[BUF_MAXSIZE];
	while(pimgdesc->install_offset < pimgdesc->img_size)
	{
		printf("\r%3f%", (float)pimgdesc->install_offset*100 / (float)pimgdesc->img_size);
		// read
		if((pimgdesc->img_size - pimgdesc->install_offset)>= sizeof(data_buf))
			rret = rtk_read_tarfile_from_urltar(&sock, data_buf, sizeof(data_buf));
		else
			rret = rtk_read_tarfile_from_urltar(&sock, data_buf, (pimgdesc->img_size - pimgdesc->install_offset));

		if(rret <= 0)
		{
			install_debug("install_offset:%u\r\n", pimgdesc->install_offset);
			goto ERR;
		}

		// write
		wret = write(fd, data_buf, rret);
		if(wret != rret || wret <=0)
		{
			install_debug("install_offset:%u wret:%d rret:%d\r\n", pimgdesc->install_offset, wret, rret);
			goto ERR;
		}
		pimgdesc->install_offset = pimgdesc->install_offset + rret;
		if(pimgdesc->install_offset > show_dot)
		{
			show_dot = show_dot + 1024*1024;
			printf(".");
		}
	}//end while
#endif
	rtk_close_tarfile_from_urltar(&sock);
	close(fd);
	if(ret != pimgdesc->img_size)
	{
		install_debug("fd_to_fd()=%d\r\n", ret);
		return -1;
	}
	printf("OK!!OK!!OK!!\r\n");fflush(stdout);
	return 0;
}

