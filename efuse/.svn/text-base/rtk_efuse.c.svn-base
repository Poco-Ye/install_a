#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include <io.h>
#include <rtk_efuse.h>
#include <MCPControl/inc/mcp_api.h>
#include <rtk_def.h>
#include <rtk_common.h>

/***************************************************/
/* otp register definition                                          */
/***************************************************/
//efuse bit
#define OTP_K_RSA				0
#define OTP_K_S                 2048
#define OTP_K_H					2304
#define OTP_K_N                 2560
#define OTP_CHIPID      		3360
#define OTP_BIT_SECUREBOOT		3494
#define OTP_OFST_KEYSEL_0		3488
#define OTP_OFST_KEYSEL_1		3489
#define OTP_SECURE_BOOT			3494
#define OTP_RMA_ENABLE0			3507
#define OTP_RMA_ENABLE1			3508
#define OTP_RMA_ENABLE2			3509
#define OTP_RMA_ENABLE3			3510
#define OTP_BOOT_ENC			3515
#define OTP_USB2SRAM			3522
#define OTP_RESV_MP_TEST1		3523	// 7-bit
#define OTP_RESV_MP_TEST2       3594	// 1-bit
#define OTP_HWSET_CHK_0         3595
#define OTP_HWSET_CHK_1         3596
#define OTP_LOAD_SECURE_OS      3598
#define OTP_DIS_ONE_CORE        3599
#define OTP_BIT_KDBG			2816
#define OTP_DUMMY_RD_REG        4064

#define OTP_PRO_EFUSE_ST     (1<<16)
#define EF_Rd_done_bit			(1<<1)

//#define EFUSE_DBG
//#define KEY_DBG

/////////////////////////////////////////////////////////////////////
//  programing eFuse API
/////////////////////////////////////////////////////////////////////

#define UIO_DEV "/dev/uio250"
#define UIO_SIZE "/sys/class/uio/uio250/maps/map0/size"

#define OTP_REG_OFFSET		0x17000
#define SYNC_REG_OFFSET		0x1a020
//regs
#define OTP_EF_CTRL				(otp_reg_base+0x400)
#define OTP_EF_CTRL_ST			(otp_reg_base+0x404)
#define OTP_EF_RD_DONE			(otp_reg_base+0x410)
#define OTP_DATA				(otp_reg_base + 0)

#define SYNC_ADDR
static int uio_size;
static unsigned int otp_reg_base;
static unsigned int sync_reg;

void rtk_sync()
{
    CP15DMB;
    rtd_outl(sync_reg, 0x0);
    CP15DMB;
}

int otp_init(void)
{
    int uio_fd, size_fd;
    char uio_size_buf[16];
    void *mapBuf;
    unsigned int reg_val;

    size_fd = open( UIO_SIZE, O_RDONLY );
    uio_fd = open( UIO_DEV, O_RDWR | O_SYNC);

    if( (size_fd < 0) || (uio_fd < 0) )
    {
        printf("%s, open failed, uio_fd(%d) size_fd(%d)\n", __func__, uio_fd, size_fd);
        return -1;
    }

    read( size_fd, uio_size_buf, sizeof(uio_size_buf) );
    uio_size = (int)strtol( uio_size_buf, NULL, 0 );
    mapBuf = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, 0);

    close(uio_fd);
    close(size_fd);

    if( mapBuf == MAP_FAILED )
    {
        printf("%s, mmap failed\n", __func__);
        return -1;
    }

    // OTP(eFuse) register base offset starts at 0x17000
    otp_reg_base = (unsigned int)((char*)mapBuf + OTP_REG_OFFSET);
    sync_reg = (unsigned int)((char*)mapBuf + SYNC_REG_OFFSET);

    return 0;
}

int otp_uninit(void)
{
    return munmap((void*)(otp_reg_base-OTP_REG_OFFSET), uio_size);
}

static UINT32 otp_judge_bit(UINT32 offset)
{
    UINT32 div_n=0, rem_n=0;
    UINT32 align_n=0, align_rem_n=0, real_n=0;

    rem_n = offset%8;
    div_n = offset/8;

    align_n = div_n & ~0x3;
    align_rem_n = div_n & 0x3;

    real_n = rtd_inl(otp_reg_base + align_n);
    return(((real_n >>(align_rem_n*8)) >> rem_n)&1);
}

static int otp_program_end()
{
    unsigned int reg_val;

#ifdef EFUSE_DBG
    //return;
#endif

    //1. set program addr to reg
    reg_val = rtd_inl(OTP_EF_CTRL);
    reg_val = 0x0080a000;
    rtd_outl(OTP_EF_CTRL, reg_val);

    //udelay(0, 5);
    usleep(5);

#ifdef EFUSE_DBG
    printf("%s : done\n", __func__);
#endif

    return 0;
}

static int otp_program_start(int bits)
{
    int addr, offset, count;
    int retry = 0;
    int retry2 = 0;
    unsigned int reg_val;

RETRY_BURN:
    if (retry2 > 6)
    {
        printf("[WST_D] check OTP_EF_CTRL_ST is failed.");
        return -1;
    }

    //1. get the parameter
    addr = (bits >> 3) & 0x1ff;  //10bits
    offset = bits & 0x7;         //3bits

#ifdef EFUSE_DBG
    //return;
#endif

    //2.1 set program addr to reg
    rtd_outl(OTP_EF_CTRL, 0x00c02000|((offset<<10)|addr));
    //2.2 polling for pro_efuse_act = 1
    count = 0;
    while (!(rtd_inl(OTP_EF_CTRL_ST)&(OTP_PRO_EFUSE_ST)))
    {
        if (count > 10)
        {
            retry2++;
            printf("[WST_D] Retry check OTP_EF_CTRL_ST.");
            goto RETRY_BURN;
        }
        count++;
        usleep(1);
    }
    printf("[WST_D] Burn OTP_BIT%d.\r\n\r\n", bits);

    //2.3 program it
    rtd_outl(OTP_EF_CTRL, 0x00c0e000|((offset<<10)|addr));

    usleep(50);
    rtk_sync();

    otp_program_end();
    rtk_sync();

    for (count = 0; count < 5; count++)
    {
        if (!otp_judge_bit(bits))
        {
            if (retry <= 2)
            {
                printf("[WST_D] Retry burn OTP_BIT%d.\r\n\r\n", bits);
                retry++;
                goto RETRY_BURN;
            }
            else
            {
                printf("[WST_D] Burn OTP_BIT%d failed.\r\n\r\n", bits);
                return -1;
            }
        }
    }

#ifdef EFUSE_DBG
    printf("%s : done\n", __func__);
#endif

    return 0;
}

static int otp_start_program (UINT32 startbit, UINT32 totalbytes, UINT8 * src_array)
{
    UINT32 i, j;
    UINT32 real_bit=0,rem_val=0;

#ifdef EFUSE_DBG
    printf("start program : start\n");
    printf("startbit = %d, totalbytes = %d\n",startbit,totalbytes);
#endif

    otp_judge_bit(OTP_DUMMY_RD_REG);

    for(i=0,j=0 ; i<totalbytes*8 ; i++,j=i/8)
    {
#ifdef EFUSE_DBG
        if ((i % 8) == 0)
            printf("\nByte[%d]: ",j);
#endif
        rem_val = i%8;
        real_bit = (*(src_array+j) >> rem_val) & 0x1;
        if (real_bit == 0x1)
        {
            if (otp_program_start(startbit+i) < 0)
                return -1;
#ifdef EFUSE_DBG
            printf("1");
#endif
        }
#ifdef EFUSE_DBG
        else
        {
            printf("0");
        }
#endif
    }
    //otp_program_end();
#ifdef EFUSE_DBG
    printf("\nstart program : end\n");
#endif

    return 0;
}

static int otp_get_byte(int offset, unsigned char *ptr, unsigned int cnt)
{
    unsigned int i=0;
    unsigned int div_n=0;
    unsigned int align_n=0, align_rem_n=0;
    unsigned int real_ofset=0;

    real_ofset = offset&~0x3;
    for(i=0; i<cnt; i++)
    {
        div_n = (real_ofset+i*8)/8;
        align_n = div_n & ~0x3;
        align_rem_n = div_n & 0x3;
        *(ptr+i) = (unsigned char)((rtd_inl(otp_reg_base + align_n)>>(align_rem_n*8))&0xff);
    }
    return cnt;
}


/////////////////////////////////////////////////////////////////////
//  key handling API
/////////////////////////////////////////////////////////////////////
static int swap_endian(unsigned int input)
{
    unsigned int output;
    output = (input & 0xff000000)>>24|
             (input & 0x00ff0000)>>8|
             (input & 0x0000ff00)<<8|
             (input & 0x000000ff)<<24;
    return output;
}

static unsigned int hexstr2int(char *s)
{
    return (unsigned int) strtoul (s, (char **) 0, 16);
}

static int bin2ascii(char *ascii, unsigned char *bin, int size)
{
    char table[] = { '0','1','2','3','4','5','6','7','8',
                     '9','a','b','c','d','e','f'
                   };

    for( int i=0; i<size; i++ )
    {
        unsigned char hi = (bin[i] & 0xf0) >> 4;
        unsigned char low = bin[i] &0x0f;

        ascii[2*i] = table[hi];
        ascii[2*i+1] = table[low];
    }

    return 0;
}

int read_aes_key(const char *filename, unsigned int key[4])
{
    unsigned int j;
    int ret, fp, filelen;
    char piece[16];
    unsigned char key_encrypted[16];
    unsigned char key_decrypted[16];
    unsigned char AES_Key[16] =
    {
        0x3d, 0xa0, 0x23, 0x88,	0xc8,
        0xa1, 0x53, 0x5f, 0x79, 0x8d,
        0x60, 0xf2, 0xd0, 0xc8, 0xa8,
        0xab
    };

    memset(key_encrypted, 0, sizeof(key_encrypted));
    memset(key_decrypted, 0, sizeof(key_decrypted));

    fp = open(filename, O_RDONLY);
    if (fp < 0)
    {
        printf("Open File(%s)error\n", filename);
        return -1;
    }
    else
    {
        filelen = lseek(fp, 0, SEEK_END);
        if( filelen > sizeof(key_encrypted) )
        {
            printf("file length(%d) not correct!\n", filelen);
            return -1;
        }
        lseek(fp, 0, SEEK_SET);
        read(fp, key_encrypted, filelen);
        close(fp);
    }

    rtk_sync();
    // key_encrypted[] is in binary format packed into install.img.
    // key_decrypted[] is in binary format.
    MCP_AES_ECB_DataDecryption(AES_Key, key_encrypted, key_decrypted, sizeof(key_decrypted) );
    rtk_sync();
#ifdef KEY_DBG
    printf("---------- decrypted aes key (%s)----------\n", filename);
    for( j=0; j<sizeof(key_decrypted); j++ )
        printf("%02x,", key_decrypted[j]);
    printf("\n---------- decrypted aes key ----------\n");
#endif

    memset(piece, 0, sizeof(piece));
    for (j = 0; j < 4; j++)
    {
        ret = bin2ascii( piece, &key_decrypted[4*j], 4 );
        if (ret == -1)
        {
            printf("Wrong input key(%s), must be hex format \n", filename);
            return -1;
        }
        key[j] = hexstr2int(piece);
    }

    //check all zero case
    if (key[0]==0 && key[1]==0 && key[2]==0 && key[3]==0)
    {
        printf("Wrong input key(%s), can't be all zero \n", filename);
        return -1;
    }
#ifdef KEY_DBG
    printf("aes:0x%08x, 0x%08x, 0x%08x, 0x%08x\n", key[0],key[1],key[2],key[3]);
#endif
    return 0;
}

int read_rsa_key(const char *filename, unsigned int key[64])
{
    unsigned int j;
    int fp, ret;
    char piece[16];
    unsigned char rsa_key[256];

    memset(rsa_key, 0, sizeof(rsa_key));

    fp = open(filename, O_RDONLY);

    if (fp < 0)
    {
        printf("Open File(%s)error\n", filename);
        return -1;
    }
    else
    {
        ret = read(fp, rsa_key, sizeof(rsa_key));
        close(fp);
    }
    if (ret != sizeof(rsa_key))
    {
        printf("rsa_key length invalid\n");
        return -1;
    }

    // rsa_key[] is in binary format.
    memset(piece, 0, sizeof(piece));
    for (j = 0; j < 64; j++)
    {
        ret = bin2ascii( piece, &rsa_key[4*j], 4 );
        if (ret == -1)
        {
            printf("Wrong input key(%s), must be hex format \n", filename);
            return -1;
        }
        key[j] = hexstr2int(piece);
        key[j] = swap_endian(key[j]);
    }

#ifdef KEY_DBG
    printf("---------- rsa key ----------\n");
    for( j=0; j<64; )
    {
        printf("0x%08x, ", key[j]);
        if( ((++j)%4) == 0 )
            printf("\n");
    }
    printf("---------- rsa key ----------\n");
#endif

    return 0;
}

int read_efuse_status(int *secure_boot, int *key_installed)
{
    *secure_boot = otp_judge_bit(OTP_SECURE_BOOT);

    if( *secure_boot == 0 )
    {
        unsigned char key_buf[(2048+256*3) >> 3];

        memset( key_buf, 0, sizeof(key_buf) );
        otp_get_byte(OTP_K_RSA, key_buf, sizeof(key_buf) );
        for( unsigned int i=0; i<sizeof(key_buf); i++ )
        {
            if( key_buf[i] )
            {
                *key_installed = 1;
                break;
            }
        }
        *key_installed = 0;
    }
    else
    {
        *key_installed = 1;
    }

    return 0;
}

int do_burn_efuse_bit (unsigned int mask)
{
    printf("do_burn_efuse_bit(), mask(%#x)\n", mask);

    otp_judge_bit(OTP_DUMMY_RD_REG);

    if( mask & EFUSE_BIT_SECURE_BOOT )
    {
#ifdef EFUSE_DBG
        printf("\nstart program : secure_boot=%d\n", OTP_SECURE_BOOT);
#endif
        if (otp_program_start(OTP_SECURE_BOOT) < 0)
            goto BURN_FAIL;
    }

    if( mask & EFUSE_BIT_BOOT_ENC )
    {
#ifdef EFUSE_DBG
        printf("\nstart program : boot_enc=%d\n", OTP_BOOT_ENC);
#endif
        if (otp_program_start(OTP_BOOT_ENC) < 0)
            goto BURN_FAIL;
    }

    if( mask & EFUSE_BIT_HWCHK_FIRST )
    {
#ifdef EFUSE_DBG
        printf("\nstart program : hwchk bit0=%d\n", OTP_HWSET_CHK_0);
#endif
        if (otp_program_start(OTP_HWSET_CHK_0) < 0)
            goto BURN_FAIL;

        if (otp_program_start(OTP_HWSET_CHK_1) < 0)
            goto BURN_FAIL;
    }

    if( mask & EFUSE_BIT_HWCHK_LATER )
    {
#ifdef EFUSE_DBG
        printf("\nstart program : hwchk bit1=%d\n", OTP_HWSET_CHK_1);
#endif
        if (otp_program_start(OTP_HWSET_CHK_1) < 0)
            goto BURN_FAIL;
    }

    //otp_program_end();
    return 0;

BURN_FAIL:
    //otp_program_end();
    return -1;
}

int do_burn_efuse_key(EFUSE_KEY key, UINT32 totalbytes, UINT8 * src_array)
{
    UINT32 offset;
    int ret;
    switch(key)
    {
        case EFUSE_KEY_RSA:
            offset = OTP_K_RSA;
            break;
        case EFUSE_KEY_H:
            offset = OTP_K_H;
            break;
        case EFUSE_KEY_N:
            offset = OTP_K_N;
            break;
        case EFUSE_KEY_S:
            offset = OTP_K_S;
            break;
    }
    ret = otp_start_program( offset, totalbytes, src_array);
    return ret;
}

int otp_key_verify()
{
    unsigned char ks[16];
    unsigned char kh[16];
    unsigned char key_img[16];
    int key_size = sizeof(ks)/sizeof(ks[0]);

    unsigned char file_encrypted[50];
    unsigned char file_decrypted[50];

    char gold_string[] = "Welcome to RTK!";	//golden string, the string of decrpted file is need to match it
    int string_size = sizeof(gold_string)/sizeof(gold_string[0]);

    int idx = 0;
    FILE *fd = NULL;

    //step 1. get the otp key, ks and kh
    //ks, kh must be byte-align
    for(idx = 0; idx < key_size; idx++)
    {
        ks[idx] = *((unsigned char *)(otp_reg_base + (OTP_K_S >> 3) + idx));
        kh[idx] = *((unsigned char *)(otp_reg_base + (OTP_K_H >> 3) + idx));
        //printf("ks[%d] = 0x%02x,\t kh[%d] = 0x%02x\n", idx, ks[idx], idx, kh[idx]);
    }

    //step 2.1 open the encrypt file
    fd = fopen(OTP_KEY_VERIFY_TEMP, "r");
    if(fd == NULL)
    {
        printf("Can't open %s\n", OTP_KEY_VERIFY_TEMP);
        return 0;
    }
    else
    {
        //step 2.2 copy the encrypted file content to variable, file_encrypted
        idx = 0;
        char ch = fgetc(fd);
        while( (ch != EOF) && (idx < string_size) )
        {
            file_encrypted[idx] = ch;
            //printf("file encrypted[%d] = %02x\n", idx, file_encrypted[idx]);
            idx++;
            ch = fgetc(fd);
        }
        fclose(fd);

        //step 3. use ks and kh to generate the key_img,
        //then use key_img as the key to decrypt the encrypted content
        MCP_AES_ECB_DataEncryption(kh, ks, key_img, sizeof(key_img));
        MCP_AES_ECB_DataDecryption(key_img, file_encrypted, file_decrypted, sizeof(file_decrypted));

        //step 4. compare the decrypted content with golden string
        //put teriminal symbol since we want to compare string (file_decrypted vs gold_string)
        file_decrypted[string_size-1] = '\0';
        //printf("key verify, decrypt string = %s\n", file_decrypted);
        //printf("key verify, match string = %s\n", gold_string);
        if(strcmp(gold_string, (const char *)file_decrypted) != 0)
        {
            printf("cmp decrypt string fail\n");
            return 0;
        }
    }
    return 1;
}

