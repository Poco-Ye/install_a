#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <rtk_common.h>
#include <rtk_urltar.h>
#include <rtk_fwdesc.h>
#include <rtk_imgdesc.h>
#include <rtk_mtd.h>
#define HTTP_CMD_MAXSIZE 512
#define FILESIZELEN 11

char HTTP_GET_RANGE_CMD[]=
"GET %s HTTP/1.1\r\n"
"Host: %s\r\n"
"X-REMOVED: Range\r\n"
"Connection: Keep-Alive\r\n"
"Range: bytes=%d-%d\r\n"
"\r\n";
char HTTP_GET_SIZE_RANGE_CMD[]=
"GET %s HTTP/1.1\r\n"
"Host: %s\r\n"
"X-REMOVED: Range\r\n"
"Connection: Keep-Alive\r\n"
"Range: bytes=0-0\r\n"
"\r\n";
char HTTP_GET_CMD[]=
"GET %s HTTP/1.1\r\n"
"Host: %s\r\n"
"Connection: Keep-Alive\r\n"
"\r\n";
enum RESPONSE_CODE
{
	CODE_PARTIAL_CONTENT=0,
	CODE_OK
};
static char HTTP_RESPONSE_CODE[][128]={
"HTTP/1.1 206 Partial Content\r\n",
"HTTP/1.1 200 OK\r\n"};

// return -1 if fail
// return 0 if connection close
// return >0 if success
static int readn(int fd, void* vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = (char*) vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
//	printf("\r\n");fflush(stdout);

	return(n - nleft);		/* return >= 0 */
}
static int writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char* ptr;

	ptr = (const char*) vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
// return -1 if fail
// reutrn value > 0 if success
static int send_httprequest(int* sock, const char* method)
{
	install_log("\r\nSEND HEADER:\r\n%s", method);
	return writen(*sock, (const void*) method, strlen(method));
}
// return -1 if fail
// return 0 if connection close
// return length of read > 0 if success
static int recv_httpresponse(int* psock, enum RESPONSE_CODE eexpectcode, char* precv_header)
{
	char recv_header[256];
	char* response;
	int ret;
	int state;
	char c;
	char* precv;

	if(NULL != precv_header)
	{
		precv = precv_header;
	}
	else
	{
		precv = &recv_header[0];
	}
	
	// get response msg until string "\r\n"
	state = 0;
	response = precv;
	do
	{
		ret = readn(*psock, (void*) &c, 1);
		if(ret < 0)
		{
			install_debug("readn() fail, errno:%d (%s)\r\n", errno, strerror(errno));
			goto error;
		}
		else if(ret == 0)
		{
			response[precv-response] = 0;
			install_debug("readn() connection close, response:%s\r\n", response);
			return 0;
		}
		*precv++ = c;
		switch(state)
		{
			case 0:
				if('\r' == c) state = 1;
				break;
			case 1:
				state = ('\n' == c) ? 2:0;
				break;
			case 2:
				state = ('\r' == c) ? 3:0;
				break;
			case 3:
				state = ('\n' == c) ? 4:0;
				break;
			default:
				state = 0;
			break;
		}
		if(4 == state)
		{
			break;
		}
	} while(1);
	*precv = 0;

	// check if the response is expected or not
	if(strncmp(response, HTTP_RESPONSE_CODE[eexpectcode], strlen(HTTP_RESPONSE_CODE[eexpectcode])))
	{
		install_debug("response code is not expected, [%s] is expected\r\n", HTTP_RESPONSE_CODE[eexpectcode]);
		goto error;
	}

	install_info("\r\nRECV HEADER:\r\n%s", response);
	return strlen(response);
error:
	install_debug("RECV HEADER:\r\n%s\r\n", response);
	return -1;
}
static int rtk_connect(int* psock, struct url* purl)
{
	int i;
    struct hostent *he;
    struct in_addr **addr_list;
	char cmd[128];
	

	if(-1 == (*psock = socket(AF_INET, SOCK_STREAM, 0)))
	{
		install_debug("Can't create socket\r\n");
		return -1;
	}
    struct sockaddr_in server_sockaddr;
    bzero(&server_sockaddr, sizeof(struct sockaddr_in));
    server_sockaddr.sin_family = AF_INET;
	if ((he = gethostbyname(purl->host)) == NULL)
	{  // get the host info
		install_debug("gethostbyname() fail:%s\r\n", purl->host);
		return -1;
	}
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
	{
        printf("Try connect to IP:%s\r\n", inet_ntoa(*addr_list[i]));

		sprintf(cmd, "ifconfig;route;ping %s", inet_ntoa(*addr_list[i]));
		rtk_command(cmd, __LINE__, __FILE__);

		server_sockaddr.sin_addr = *addr_list[i];
		//server_sockaddr.sin_addr.s_addr = inet_addr(purl->host);
		server_sockaddr.sin_port = htons(purl->port);
		// FIXME: no timeout
		if(connect(*psock, (struct sockaddr*) &server_sockaddr, sizeof(server_sockaddr)) < 0)
		{
			install_debug("Can't connect to [socket:%d][addr:%s(%08X)][port:%d][path:%s]\r\n"
			, *psock
			, inet_ntoa(*addr_list[i])
			, *addr_list[i]
			, purl->port
			, purl->path);
			perror(NULL);
		}
		else
		{
			printf("Connect to %s:%d:%s\r\n"
			, inet_ntoa(*addr_list[i])
			, purl->port
			, purl->path);

			return 0;
		}
	}
	return -1;
}
// psock : IN_OUT we will connect it if *psock below zero
// purl : IN
// get_method : IN
// eCode : IN
// ret_val > 0 if connection success
// return 0 if connection close
// ret_val < 0 if connection fail
static int rtk_connect_http_get_range(int* psock, struct url* purl, const char* get_method, enum RESPONSE_CODE eCode, char* precvheader)
{
	int ret;
	// connect
	if(*psock < 0)
	{
		ret = rtk_connect(psock, purl);
		if(ret < 0)
		{
			install_debug("rtk_connect fail\r\n");
			return -1;
		}
	}
	// send msg
	ret = send_httprequest(psock, get_method);
	if(0 > ret)
	{
		install_debug("sendrequest():%d\r\n", ret);
		return -1;
	}
	// recv msg
	ret = recv_httpresponse(psock, eCode, precvheader);
	if(0 >= ret)
	{
		install_debug("recvresponse():%d\r\n", ret);
		return ret;
	}

    return ret;
}

// sock : IN
// buf : INOUT
// count : IN
static int rtk_readn_http_get(int* sock, void* buf, size_t count)
{
	return readn(*sock, buf, count);
}
// purl->tarfile_size : INOUT
// purl->path, purl->host, purl->port : IN
int rtk_get_size_of_tarfile_from_urltar(struct url* purl)
{
	int http_s;
	int ret;
	char http_get_cmd[HTTP_CMD_MAXSIZE];
	char response[HTTP_CMD_MAXSIZE];
	char* str;
	
	// initial input arg
	sprintf(http_get_cmd, HTTP_GET_SIZE_RANGE_CMD, purl->path, purl->host);
	memset((void*) response, 0, sizeof(response));
	http_s = -1;
	ret = rtk_connect_http_get_range(&http_s, purl, http_get_cmd, CODE_PARTIAL_CONTENT, response);
	// if ret = 0, it's ok. The connection close is normal.
	if(ret < 0)
	{
		install_debug("rtk_connect_http_get_range() fail\r\n");
		goto ERR;
	}
	if(NULL == (str = strstr(response, "Content-Range")))
	{
		install_debug("Can't find Content-Range pattern\r\n");
		goto ERR;
	}
	sscanf(str, "Content-Range: bytes 0-0/%u", &purl->tarfile_size);
	close(http_s);
	return 0;
ERR:
	close(http_s);
	return -1;
}
// purl->tarfile_offset : IN
// purl->path, purl->host, purl->port : IN
// ptarhead : OUT
// return length of recv header if success
// return 0 if connection close
// return -1 if connection error
int rtk_get_tarhead_from_urltar(int* phttp_s, struct url* purl, struct t_tarheader* ptarhead)
{
	int ret;
	char http_get_cmd[HTTP_CMD_MAXSIZE];
	char response[HTTP_CMD_MAXSIZE];
	
	// *phttp_s = -1;
	// initial input arg
	sprintf(http_get_cmd, HTTP_GET_RANGE_CMD, purl->path, purl->host, purl->tarfile_offset, purl->tarfile_offset+sizeof(struct t_tarheader)-1);
	memset((void*) response, 0, sizeof(response));
	ret = rtk_connect_http_get_range(phttp_s, purl, http_get_cmd, CODE_PARTIAL_CONTENT, response);
	if(ret <= 0)
	{
		install_debug("rtk_connect_http_get_range() fail, ret=%d\r\n", ret);
		goto ERR;
	}
	ret = rtk_readn_http_get(phttp_s, (void*) ptarhead, sizeof(struct t_tarheader));
	if(ret <= 0 || sizeof(struct t_tarheader) != ret)
	{
		install_debug("rtk_readn_http_get() fail:%d\r\n", ret);
		goto ERR;
	}

	//close(phttp_s);
	return ret;
ERR:
	close(*phttp_s);
	return ret;
}
// sock : INOUT
// ppath : IN
// tatfile_offset : IN
// return 0 if connection close
// return < 0 if connection error
// return > 0 if connection susscee
int rtk_open_tarfile_from_localtar(int* phttp_s, char* ppath, unsigned int tarfile_offset, unsigned int tarfile_install, unsigned int img_size)
{
return 0;
}
// sock : INOUT
// purl->path, purl->host, purl->port : IN
// tatfile_offset : IN
// return 0 if connection close
// return <0 if connection error
// return >0 if connection susscee
int rtk_open_tarfile_from_urltar(int* phttp_s, struct url* purl, unsigned int tarfile_offset, unsigned int tarfile_install, unsigned int img_size)
{
	int ret;
	char http_get_cmd[HTTP_CMD_MAXSIZE];
	char response[HTTP_CMD_MAXSIZE];
	
	// initial input arg
	sprintf(http_get_cmd, HTTP_GET_RANGE_CMD, purl->path, purl->host, tarfile_offset+512+tarfile_install, tarfile_offset+512+img_size);
	memset((void*) response, 0, sizeof(response));
	*phttp_s = -1;
	ret = rtk_connect_http_get_range(phttp_s, purl, http_get_cmd, CODE_PARTIAL_CONTENT, response);
	if(ret <= 0)
	{
		install_debug("Can't open tarfile, url:%s, ret=%d\r\n", purl->url, ret);
	}
	install_info("Open tarfile:%s offset:%u\r\n", purl->url, tarfile_offset+512+tarfile_install);
	return ret;
}
// sock:IN
int rtk_read_tarfile_from_urltar(int* phttp_s, void* buf, size_t count)
{
	return readn(*phttp_s, buf, count);
}
// sock:IN
int rtk_close_tarfile_from_urltar(int* psock)
{
	close(*psock);
	return 0;
}

int urlparse(const char* purlstr, struct url* purl)
{
	// sanity check
	if(0 == strlen(purlstr))
	{
		install_debug("url string is NULL\r\n");
		return -1;
	}
	sprintf(purl->url, "%s", purlstr);
	
	char* strURL;
	char strport[10]="";
	strURL = purl->url;
	printf("parsing url(%s)\r\n", strURL);
	sscanf(strURL, "http://%[^:/]s", purl->host);
	//printf("%s\r\n", strURL);
	//printf("(%c)\r\n", *(strURL+7+strlen(purl->host)));
	strURL = strURL + 7 + strlen(purl->host);

	if(':' == *strURL)
	{
		sscanf(strURL, ":%[^/]s", strport);
		purl->port = atoi(strport);
		strURL = strURL + 1 + strlen(strport);
	}
    else
    {
        purl->port = 80;
    }

    sscanf(strURL, "%s", purl->path);

	purl->tarfile_offset = 0;
	purl->tarfile_size = 0;

	install_log("host:%s port:%d path:%s\r\n", purl->host, purl->port, purl->path);
	if(!strlen(purl->host)|!strlen(purl->path))
	{
		install_debug("url(%s) : host(%s) or path(%s) is NULL\r\n", purl->url, purl->host, purl->path);
		return -1;
	}
	return 0;
}
int rtk_get_size_of_url(const char* urlstr)
{
	int ret;
	struct url _url;
	ret = urlparse(urlstr, &_url);
	if(ret < 0)
	{
		install_debug("urlparse fail");
		return -1;
	}

	ret = rtk_get_size_of_tarfile_from_urltar(&_url);
	if(ret < 0)
	{
		install_debug("rtk_get_size_of_tarfile_from_urltar fail");
		return -1;
	}

	install_log("url(%s), size=%u\r\n", _url.url, _url.tarfile_size);
	return _url.tarfile_size;
}
// either
// stored_file is not null
// or
// fd > 0
int rtk_urlwget(const char* strurl, const char* stored_file, int fd)
{
	int ret;
	int http_s, file_fd;
	char http_get_cmd[HTTP_CMD_MAXSIZE];
	char response[HTTP_CMD_MAXSIZE];
	struct url _url;

	// sanity-check
	if(((NULL == stored_file)&&(fd==-1)))
	{
		install_debug("stored_file is NULL or fd is -1");
		return -1;
	}

	ret = urlparse(strurl, &_url);
	if(ret < 0)
	{
		install_debug("urlparse fail");
		return -1;
	}
#if 1
	ret = rtk_get_size_of_tarfile_from_urltar(&_url);
	if(ret < 0)
	{
		install_debug("rtk_get_size_of_tarfile_from_urltar fail");
		return -1;
	}

	install_log("url(%s), size = %u B = %u KB = %u MB\r\n", _url.url, _url.tarfile_size, _url.tarfile_size>>10, _url.tarfile_size>>20);
#endif
	// initial input arg
	// source fd
	sprintf(http_get_cmd, HTTP_GET_CMD, _url.path, _url.host);
	memset((void*) response, 0, sizeof(response));
	http_s = -1;
	ret = rtk_connect_http_get_range(&http_s, &_url, http_get_cmd, CODE_OK, response);
	if(ret <= 0)
	{
		install_debug("Can't open tarfile, url:%s, ret=%d\r\n", _url.url, ret);
		return -1;
	}

	// destination fd
	if(stored_file != NULL)
	{
		file_fd = open(stored_file, O_CREAT|O_WRONLY|O_TRUNC, 0644);
		printf("open %s\r\n", stored_file);
	}
	else
	{
		file_fd = fd;
		printf("assign fd to file_fd \r\n");
	}

	if(file_fd < 0)
	{
		install_debug("Can't create file:%s\r\n", stored_file);
		close(http_s);
		return -1;
	}

	ret = fd_to_fd(http_s, file_fd, _url.tarfile_size);
	close(http_s);
	close(file_fd);
	if((unsigned int)ret != _url.tarfile_size)
	{
		install_debug("fd_to_fd fail\r\n");
		return -1;
	}
	return ret;
}

