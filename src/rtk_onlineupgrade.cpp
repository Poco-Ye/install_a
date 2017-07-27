#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>


#include <rtk_mtd.h>
#include <rtk_common.h>
#include <rtk_fwdesc.h>
#include <rtk_urltar.h>
int get_nandflash_mtd_eerasesize(void)
{
	int dev_fd;
	struct mtd_info_user meminfo;
	dev_fd = rtk_open_mtd_char();
	if(-1 == dev_fd)
		return -1;

	/* Get MTD device capability structure */
	if (ioctl(dev_fd, MEMGETINFO, &meminfo))
	{
		close(dev_fd);
		return -1;
	}
	close(dev_fd);

	// TODO: here need to modify
	dev_fd = rtk_open_mtd_block();
	if(-1 == dev_fd)
		return -1;
	close(dev_fd);
	return meminfo.erasesize;
}
int onlineupgrade_by_backuparea_api(const char* url)
{
	int ret;
	int dev_fd, file_fd;
	unsigned int startAddress, endAddress;
	unsigned int file_length;
	struct stat statbuf;
	char buf[128];

	// find offset of backup area
	ret = findNandBackupAreaStartEndAddress(&startAddress, &endAddress);
	if(ret < 0)
	{
		install_debug("Can't find nand backup area\r\n");
		return -1;
	}
	startAddress = startAddress + get_nandflash_mtd_eerasesize();//rtk_get_meminfo;
	install_log("startAddress:0x%08x endAddress:0x%08x\r\n", startAddress, endAddress);
	if((endAddress-startAddress) < statbuf.st_size)
	{
		install_debug("backup area space(%u=%uKB) < install.img(%u=%uKB)\r\n", (endAddress-startAddress), (endAddress-startAddress)/1024, statbuf.st_size, statbuf.st_size/1024);
		return -1;
	}
	// get file_length and write length of install.img
	ret = rtk_get_size_of_url(url);
	if(ret < 0)
	{
		install_debug("Can't get size of url\r\n");
		return -1;
	}
	file_length = ret;

	dev_fd = rtk_open_mtd_block_with_offset(startAddress-4096);
	if(dev_fd < 0)
	{
		install_debug("Can't open mtdblock\r\n");
		return -1;
	}

	ret = write(dev_fd, &file_length, 4);
	if(ret != 4)
	{
		install_debug("Can't write filelen\r\n");
		close(dev_fd);
		return -1;
	}

	// write data
	ret = lseek(dev_fd, startAddress, SEEK_SET);
	if(ret < 0)
	{
		install_debug("Can't lseek (%u)\r\n", startAddress);
		close(dev_fd);
		return -1;
	}
#if 1
	ret = rtk_urlwget(url, NULL, dev_fd);
	if(ret < 0)
	{
		install_debug("rtk_urlwget fail (%d)\r\n", ret);
		close(dev_fd);
		return -1;
	}
	close(dev_fd);
#else
	ret = stat(install_img_path, &statbuf);
	if(ret < 0)
	{
		install_debug("Can't find the install.img(%s)\r\n", install_img_path);
		return -1;
	}
	install_log("install.img:%s (len=%u)\r\n", install_img_path, statbuf.st_size);
	file_fd = open(install_img_path, O_RDONLY);
	if(file_fd < 0)
	{
		install_debug("Can't open file (%s)\r\n", install_img_path);
		close(dev_fd);
		return -1;
	}
	ret = fd_to_fd(file_fd, dev_fd, statbuf.st_size);
	close(dev_fd);
	close(file_fd);
	if(ret != statbuf.st_size)
	{
		install_debug("Burn fail (%d=fd_to_fd())\r\n", ret);
		return -1;
	}

#endif

	sprintf(buf, "echo dd://dev/mtdblock/0/%X > /tmp/installing_source", startAddress);
	ret = rtk_command(buf, __LINE__, __FILE__);
	if(ret < 0)
	{
		install_debug("rtk_command fail:%d\r\n", ret);
		return -1;
	}
	return 0;
}

