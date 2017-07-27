#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <rtk_common.h>
#include <rtk_imgdesc.h>
#include <rtk_burn.h>
#include <rtk_xml.h>
// tinyxml
#include <tinyxml.h>

static TiXmlElement* packageFileElement;
static TiXmlElement* packageElement;
static TiXmlElement* infoElement;
static TiXmlElement* nandElement;
static TiXmlElement* norElement;
#define CONFIG_PATH "/tmp/configuration.xml"
static int load_xml_file(struct t_rtkimgdesc* prtkimgdesc);
static TiXmlElement* search_image_element(TiXmlElement *_nandElement, const char* type, const char* mount_point);

const char* get_linuxKernel_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "linuxKernel", NULL);
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;

	return fileName;
}

const char* get_tee_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "tee", NULL);
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;

	return fileName;
}
const char* get_audioKernel_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "audioKernel", NULL);
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;

	return fileName;
}

const char* get_videoKernel_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "videoKernel", NULL);
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;

	return fileName;
}

const char* get_etc_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, NULL, "/usr/local/etc");
	printf("etcElement:%08x", etcElement);
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;
	install_info("Got etc's filename(%s)\r\n", fileName!=NULL ? fileName:"none");

	return fileName;
}

const char* get_rootfs_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "squash", "/");
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;
	install_info("Got rootfs's filename(%s)\r\n", fileName!=NULL ? fileName:"none");

	return fileName;
}

const char* get_resource_filename(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "squash", "/usr/local/Resource");
	TiXmlElement* fileNameElement = (etcElement!=NULL) ? etcElement->FirstChildElement("fileName") : NULL;
    const char *fileName = (fileNameElement!=NULL) ? fileNameElement->GetText(): NULL;

	return fileName;
}

long long int get_etc_partition_size(struct t_rtkimgdesc* prtkimgdesc)
{
    int ret;
    ret = load_xml_file(prtkimgdesc);
    if(ret < 0)
    {
        install_debug("Can't load configuration.xml\r\n");
        return NULL;
    }
    int DEFAULT_ETC_SIZE;
    DEFAULT_ETC_SIZE = 0x40000;

    TiXmlElement* etcElement = search_image_element(nandElement, NULL, "/usr/local/etc");
    TiXmlElement* sizeBytesMinElement = (etcElement!=NULL) ? etcElement->FirstChildElement("sizeBytesMin") : NULL;
    const char *strsizeBytesMin = (sizeBytesMinElement!=NULL) ? sizeBytesMinElement->GetText(): NULL;
    long long sizeBytesMin = (strsizeBytesMin!=NULL)?strtoll(strsizeBytesMin, NULL, 0):DEFAULT_ETC_SIZE;
    return sizeBytesMin;
}

long long int get_preserve_partition_size(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "yaffs2", "/tmp/preserve");
	TiXmlElement* sizeBytesMinElement = (etcElement!=NULL) ? etcElement->FirstChildElement("sizeBytesMin") : NULL;
    const char *strsizeBytesMin = (sizeBytesMinElement!=NULL) ? sizeBytesMinElement->GetText(): NULL;
	long long sizeBytesMin = (strsizeBytesMin!=NULL)?strtoll(strsizeBytesMin, NULL, 0):-1;
	return sizeBytesMin;
}

long long int get_cache_partition_size(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	TiXmlElement* etcElement = search_image_element(nandElement, "yaffs2", "cache");
	TiXmlElement* sizeBytesMinElement = (etcElement!=NULL) ? etcElement->FirstChildElement("sizeBytesMin") : NULL;
    const char *strsizeBytesMin = (sizeBytesMinElement!=NULL) ? sizeBytesMinElement->GetText(): NULL;
	long long sizeBytesMin = (strsizeBytesMin!=NULL)?strtoll(strsizeBytesMin, NULL, 0):-1;
	return sizeBytesMin;
}

// 0: fail or nand flash
// 1: spi
int check_target_flash_is_spi(void) {
    int fd = 0;
    int ret;
    char sys_parameters[256];
    char *ptr, *ptr1;
    char *sep=": \t\n";

#define SYSTEM_PARAMETERS   "/sys/realtek_boards/system_parameters"

    memset(sys_parameters, 0 , sizeof(sys_parameters));

    fd = open(SYSTEM_PARAMETERS, O_RDONLY);

    if((fd = open(SYSTEM_PARAMETERS, O_RDONLY, 0644)) == -1) {
        install_log("ERROR: open %s error.(%s)\n", SYSTEM_PARAMETERS, strerror(errno));
        //sninstall_log(threadmsg, sizeof(threadmsg), "Open %s error!",SYSTEM_PARAMETERS);
        return 0;
    }

    if((ret = read(fd, (char*)sys_parameters, sizeof(sys_parameters))) < 0) {
        install_log("ERROR: can not read system parameters.(%s)\n", strerror(errno));
        //sninstall_log(threadmsg, sizeof(threadmsg), "system parameters read error!");
        close(fd);
        return 0;
    }

    //install_log("#@#[cfyeh-debug] %s(%d) sys_parameters %s\n", __func__, __LINE__, sys_parameters);
    ptr = strstr(sys_parameters, "boot_flash");
    if(!ptr) {
        close(fd);
        return 0;
    }

    ptr = strtok(sys_parameters, sep);
    ret = 0;
    while(ptr){
        ptr = strtok(NULL, sep);
        if(!ptr)
            continue;
        //install_log("#@#[cfyeh-debug] %s(%d) ptr %s\n", __func__, __LINE__, ptr);
        if(strstr(ptr, "boot_flash")) {
            ptr1 = strstr(ptr, "=");
            ptr1++;
            if(strstr(ptr, "spi"))
                ret = 1;
        }
    }

    close(fd);

    return ret;
}

int check_secure_boot(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}

	const char *package_attr_str = packageElement->Attribute("secureboot");

	/* Check image type. */
	if(package_attr_str == NULL)
		return 0;

	if(strcmp(package_attr_str, "y") == 0)
		return 1;
}

char ID_str_mapping[][20]={"linuxKernel", "audioKernel", "videoKernel"};
const char* get_kernel_RSA_sign_value(struct t_rtkimgdesc* prtkimgdesc, enum firm_ID eID)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}


	TiXmlElement* kernelElement = search_image_element(nandElement, ID_str_mapping[eID], NULL);
	TiXmlElement* RSA_sign_Element = kernelElement->FirstChildElement("marssha1_rsa_hashValue");
	const char *RSA_sign_str = (RSA_sign_Element!=NULL) ? RSA_sign_Element->GetText(): NULL;
	return RSA_sign_str;
}

const char* get_rootfs_RSA_sign_value(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}


	TiXmlElement* kernelElement = search_image_element(nandElement, "squash", "/");
	TiXmlElement* RSA_sign_Element = kernelElement->FirstChildElement("marssha1_rsa_hashValue");
	const char *RSA_sign_str = (RSA_sign_Element!=NULL) ? RSA_sign_Element->GetText(): NULL;
	return RSA_sign_str;
}

const char* get_version(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}
    TiXmlElement* versionElement = (infoElement!=NULL) ? infoElement->FirstChildElement("version") : NULL;
    const char *strValue = (versionElement!=NULL) ? versionElement->GetText(): NULL;
	return strValue;
}

int checkinstallbootcode(struct t_rtkimgdesc* prtkimgdesc)
{
	int ret;
	ret = load_xml_file(prtkimgdesc);
	if(ret < 0)
	{
		install_debug("Can't load configuration.xml\r\n");
		return NULL;
	}
    TiXmlElement* installerAPElement;
    TiXmlElement* installbootElement;
    const char* install_boot;
    // condiction to decide if bootcode need to be installed
    installerAPElement = (packageFileElement!=NULL) ? packageFileElement->FirstChildElement("installerAP") : NULL;
    installbootElement = (installerAPElement!=NULL) ? installerAPElement->FirstChildElement("install_boot") : NULL;
    install_boot = (installbootElement!=NULL) ? installbootElement->GetText(): NULL;

    if((install_boot!=NULL)&&!strcmp(install_boot, "y")) {
        return 1;
    }
    return 0;
}

static int load_xml_file(struct t_rtkimgdesc* prtkimgdesc)
{
	static bool isload = false, is_success;
	static TiXmlDocument doc;
	static TiXmlHandle docHandle(&doc);
	struct stat st;
	if(isload)
	{
		return is_success ? 0:-1;
	}

	if(stat(CONFIG_PATH, &st) == 0)
	{
		install_log("Found %s\r\n", CONFIG_PATH);
	}
	else
	{
		if(rtk_extract_file(prtkimgdesc, &prtkimgdesc->configuration, CONFIG_PATH) < 0)
		{
			install_log("Can not extract configuration document, %s!\n", CONFIG_PATH);
			return -1;
		}
	}

	if(!doc.LoadFile(CONFIG_PATH))
	{
		install_log("Can not load configuration document, %s!\n", CONFIG_PATH);
		isload = true; is_success = false;
		return -1;
	}

	doc.Print(stdout);
	packageFileElement = docHandle.FirstChildElement("packageFile").ToElement();
    packageElement = (packageFileElement!=NULL) ? packageFileElement->FirstChildElement("package") : NULL;
    nandElement = (packageElement!=NULL) ? packageElement->FirstChildElement("nand") : NULL;
    norElement = (packageElement!=NULL) ? packageElement->FirstChildElement("flash") : NULL;
	isload = true; is_success = true;

	return 0;
}

static TiXmlElement* search_image_element(TiXmlElement *_nandElement, const char* type, const char* mount_point)
{
    TiXmlElement* imageElement;

	if(_nandElement == NULL)
	{
		install_debug("Can not find nor or nand tag\r\n");
		return NULL;
	}

	if((imageElement = nandElement->FirstChildElement("image")) == NULL)
	{
		install_info("Empty nand!, try nor\n");
    }
	if((imageElement = norElement->FirstChildElement("image")) == NULL)
	{
		install_debug("Empty nor!, xml fail\n");
		return NULL;
    }

    for(;imageElement; imageElement = imageElement->NextSiblingElement("image"))
	{
        const char *image_type_str = imageElement->Attribute("type");
        TiXmlElement *mountPointElement = imageElement->FirstChildElement("mountPoint");
		const char *image_mount_point_str = (mountPointElement!=NULL) ? mountPointElement->GetText():NULL;
        /* Check image type. */
		printf("type=%s\r\n", image_type_str);
        if(image_type_str == NULL || image_mount_point_str==NULL)
		{
            install_debug("image_type(isNULL:%d) image_mount(isNULL:%d)\n", image_type_str==NULL, image_mount_point_str==NULL);
            continue;
        }
		if(NULL == mount_point)
		{
			printf("mount_point is NULL\r\n");
			if(strcmp(image_type_str, type) == 0)
				return imageElement;
		}
		if(NULL == type)
		{
			printf("type is NULL\r\n");
			if(strcmp(image_mount_point_str, mount_point) == 0)
		    	return imageElement;
		}
        if((strcmp(image_type_str, type) == 0) && (strcmp(image_mount_point_str, mount_point) == 0))
		{
		    return imageElement;
        }
    }
	return NULL;

}

