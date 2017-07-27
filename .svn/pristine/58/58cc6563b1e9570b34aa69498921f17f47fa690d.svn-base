#ifndef RTK_URLTAR_H
#define RTK_URLTAR_H
#include <stdlib.h>


/* Default port definitions */
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_FTP_PORT 21
#define DEFAULT_HTTPS_PORT 443
/* Note: the ordering here is related to the order of elements in
   `supported_schemes' in url.c.  */
enum url_scheme {
  SCHEME_HTTP,
  SCHEME_FTP,
  SCHEME_INVALID
};

/* Structure containing info on a URL.  */
struct url
{
  char url[256];			/* Original URL */
  enum url_scheme scheme;	/* URL scheme */

  int sock;
  char host[128];			/* Extracted hostname */
  unsigned int port;			/* Port number */
  unsigned int tarfile_offset;
  unsigned int tarfile_size;
  /* URL components (URL-quoted). */
  char path[128];
  /* Extracted path info (unquoted). */
  char *dir;
  char *file;
  /* Username and password (unquoted). */
  char *user;
  char *passwd;

  /* ignore */
  char *params;
  char *query;
  char *fragment;
};
// urltar.h
struct t_tarheader
{
	char filename[100];			// offset 0
	char filemode[8];
	char ownerID[8];
	char groupID[8];
	char filesize[12];			// offset 124
	char lastmodifiedtime[12];
	char checksum[8];			// offset 148, this is not implemented
	char typeflag[1];			// offset 156
	char namelinkedfile[100];
	char ustarin[6];			// offset 257, this is not implemented
	char ver[2];
	char ownername[32];
	char groupname[32];
	char devmajmun[8];
	char devminnum[8];
	char prefix[155];
	union {
		char unknown[12];
		unsigned int tarfile_offset;
	}last;

} __attribute__((packed));
int urlparse(const char* purlstr, struct url* purl);


int rtk_open_tarfile_from_urltar(int* phttp_s, struct url* purl, unsigned int tarfile_offset, unsigned int tarfile_install, unsigned int img_size);
int rtk_read_tarfile_from_urltar(int* phttp_s, void* buf, size_t count);
int rtk_close_tarfile_from_urltar(int* psock);
int http_wget(const char* purlstr, const char* filename, const char* stored_filename);
int rtk_get_size_of_url(const char* urlstr);
int rtk_get_size_of_tarfile_from_urltar(struct url* purl);
int rtk_get_tarhead_from_urltar(int* phttp_s, struct url* purl, struct t_tarheader* ptarhead);
int rtk_urlwget(const char* url, const char* stored_file, int fd=-1);
#endif
