#ifndef _EFUSE_H_
#define _EFUSE_H_

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

#define EFUSE_BIT_SECURE_BOOT	(1<<0)
#define EFUSE_BIT_BOOT_ENC		(1<<1)
#define EFUSE_BIT_HWCHK_FIRST	(1<<2)
#define EFUSE_BIT_HWCHK_LATER	(1<<3)

enum EFUSE_KEY {
	EFUSE_KEY_RSA,
	EFUSE_KEY_H,
	EFUSE_KEY_N,
	EFUSE_KEY_S
};
/***************************************************/
/* Definition                                                                       */
/***************************************************/

int otp_init(void);
int otp_uninit(void);

int read_aes_key(const char *filename, unsigned int key[4]);
int read_rsa_key(const char *filename, unsigned int key[64]);
int read_efuse_status(int *secure_boot, int *key_installed);
int do_burn_efuse_bit (unsigned int mask);
int do_burn_efuse_key(EFUSE_KEY key, UINT32 totalbytes, UINT8 * src_array);
int otp_key_verify();
void rtk_sync();
#endif // _EFUSE_H_
