#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <Platform_Lib/MCPControl/MCPUtil.h>
#include <Platform_Lib/MCPControl/mcp_rsa.h>
#include <Platform_Lib/MCPControl/mcp_aux.h>
#include <hresult.h>


bool rtk_verifyv1(const char* package_file_path);
#define RSA_MODULOUS_REG  "/sys/realtek_boards/RSA_KEY_MODULUS"
#define RSA_MODULOUS_REG_TMP "/tmp/RSA_KEY_MODULUS"
//#define RSA_MODULOUS_REG  "mod"
#define HASH_LENGTH       20
#define BLOCK_SIZE         (512 * 1024)
#define SIGNATURE_SIZE      128
#define MCPB_BUFF_SIZE     (BLOCK_SIZE +  SIGNATURE_SIZE)



static 
int DumpFileToMemory(
    const char*             fname,         
    long                    offset, 
    int                     whence,
    unsigned char*          pbuff,
    unsigned long           size
    )
{
    FILE* fd = fopen(fname, "rb");
    int ret = 0;
    
    if (fd)
    {        
        fseek(fd, offset, whence);
        
        ret = fread(pbuff, 1, size, fd);                
        fclose(fd);
    }        
    
    return ret;
}

static
long GetFileSize(const char* fname)       
{    
    long len = 0;
    FILE* fd;
    
    if ((fd=fopen(fname, "rb")))
    {        
        fseek(fd, 0, SEEK_END);        
        len = ftell(fd);
        fclose(fd);
    }                
    
    return len;
}



#ifdef USE_FIXED_RSA_KEY

const char RSA_MOD[]="CC427A5B49757F7932AFE42E45EE3E9F557B53EE82BB30F92FD0101F7266AD848DAE66B9ABFDB7089E69006CBD2FA03C8705EDAEB1BDDB15E3C8DA60843A7C8E054E17457744403FA8A7A65A0E7928FC24ED53C6D2225B83CFECE0A3BC2C794E7B2CB9507471F9A997C9F0C42A3DC5C965BE6736CF9C0EADD20654C2B1B8827D";

#endif
 
const char RSA_JH[]="A060C4E81F626A68F696BF5AC2CAC9B9158EA5AB6FD3C00CFBB1E6DC9EE67A49D6E2A2C1AA8AC63EC1C9827C7549CFCB6818B1C82B650FBE17B51CE02679477F8B80F2FE82F7D548D002705528CABF8876DE09282F86A809F03F7CD92A6E9C6CC48F0002778D63D3E5B406020BF08E4D80C13DF7AFE2E26FEBC6AA3EBC66D3D5";
int rtk_verifyv2(const char* fname, BI* _mod)
{    
    unsigned char buff[512];
    long flen = GetFileSize(fname);
    BI* signature = NULL;
    BI* mod       = NULL;
    BI* pub_exp   = move_p(65537);    
    BI* dgst      = init_BI();
    int ret       = -1;        
   
    printf("Start Do Image Verify V2\n");    
    
    if ((flen = GetFileSize(fname)) < 128)    
        goto end_proc;    
  
    printf("Decode RSA signature\n");    
    
   //------------ get modulous ------------
    mod = _mod;

    //------------ get signature ------------        
    if (DumpFileToMemory(fname, -128, SEEK_END, buff, 128)!=128)
        goto end_proc;    
    
    signature = InPutFromAddr(buff, 128);        
    
    //------------ do ras verify ------------        
    if (pub_exp==NULL || mod==NULL || dgst==NULL || signature==NULL)    
        goto end_proc;

#ifdef RSA_VERIFY_DEBUG
    printf("<<modulous>>");
    dump_bi(mod);
    printf("<<signature>>");
    dump_bi(signature);
    printf("<<public_ext>>");
    dump_bi(signature);
#endif
 
    if (MCP_RSAVerify(signature, pub_exp, mod, dgst)!=S_OK)   
        goto end_proc;
       
   // goto end_proc;    
    printf("Compute Hash value\n");    
    
    //------------- compute hash --------------                    
    if (MCP_FileHashWithRegion(MCP_MD_MARS_SHA1, fname, buff, 512 * 1024, 0, flen-128)!=20)    
        goto end_proc;        
    
    mcp_dump_data_with_text(buff, 20, "computed hash value\n");        
    
    free_BI(signature);
    
    //------------- compare hash --------------                 
    if ((signature = InPutFromAddr(buff, 20)))                               
    {                       
#ifdef RSA_VERIFY_DEBUG
        dump_bi(signature);
        dump_bi(dgst);
#endif
        ret = (Cmp(signature, dgst)==0) ? 0 : -1;
    }            
    
end_proc:    
    
    printf("Stop Compute Hash value\n");    

    if (signature)  free_BI(signature);
    if (pub_exp)    free_BI(pub_exp);
    if (mod)        free_BI(mod);
    if (dgst)       free_BI(dgst);
    return ret;            
}


bool rtk_verifyv1(const char* package_file_path)
{
#define AES_H_FILE_HASHING_BUFFER_SIZE        (512 * 1024)
	int ret;
	FILE *fd = NULL;
	struct stat st;
	int m_imageFileSize;
	if(stat(package_file_path, &st) != 0)
	{
		printf("[%s:%d:%s], Can not find %s\n",__FILE__,__LINE__,__func__, package_file_path);
	}
	m_imageFileSize = st.st_size;
	if ((fd = fopen(package_file_path, "rb")) == NULL)
	{
		printf("[%s:%d:%s], open %s fail\n",__FILE__,__LINE__,__func__, package_file_path);
		return false;
	}
	fseek(fd, m_imageFileSize-16, SEEK_SET);

	unsigned char Hash0[16] = {0};
	unsigned char Hash1[16] = {0};
	unsigned char Hash2[16] = {0};
	unsigned char Key[16];

	//for Jupiter : this key should be 0, for Mars: all bits should be 1
	#if IS_CHIP(MARS)
	memset(Key, 0xFF, sizeof(Key));
	#else
	memset(Key, 0, sizeof(Key));
	// how about saturn???
	#endif

	//setTimeoutValue(EXTENDED_WATCHDOG_TIMEOUT_THRESHOLD*3);
	fread(Hash0, 1, 16, fd);

	for(int i=0; i<16; i++) printf("0x%2x ", Hash0[i]);
    printf("\n K: ");
    for(int i=0; i<16; i++) printf("0x%2x ", Key[i]);
    printf("\n");
    MCP_AES_ECB_DataDecryption(
	#if IS_CHIP(MARS) || IS_CHIP(JUPITER)|| IS_CHIP(SATURN)
		Key
	#else
		0
	#endif
		, Hash0, Hash1, 16);
    for(int i=0; i<16; i++) printf("0x%2x ", Hash1[i]);
    printf("\n");

    ret = MCP_AES_H_FileHash(package_file_path, Hash2, AES_H_FILE_HASHING_BUFFER_SIZE, 0, m_imageFileSize-16);
    for(int i=0; i<16; i++) printf("0x%2x ", Hash2[i]);
    printf("\n");
	fclose(fd);

    if (ret == S_OK)
    {
        for (int i=0; i<16; i++)
        {
            if (Hash1[i] != Hash2[i])
            {
				printf("[%s:%d:%s], check hash fail\n",__FILE__,__LINE__,__func__);
				//setTimeoutValue(DEFAULT_WATCHDOG_TIMEOUT_THRESHOLD);
				return false;
            }
        }
    }
    else {
		
	printf("[%s:%d:%s], MCP_AES_H_FileHash != S_OK\n",__FILE__,__LINE__,__func__);
        //setTimeoutValue(DEFAULT_WATCHDOG_TIMEOUT_THRESHOLD);
		return false;
    }
	printf("[%s:%d:%s], check hash success\n",__FILE__,__LINE__,__func__);
	return true;
}

bool rtk_verify_wrapper(const char* package_file_path)
{
	struct stat st;
	int ret;
	unsigned char buff[256];
	int len;
	BI* mod = NULL;
// for debug
	mod = InPutFromStr((char*)RSA_JH, HEX);
	printf("[JH] verify built-in:%d\r\n", rtk_verifyv2(package_file_path, mod));
    len = DumpFileToMemory(RSA_MODULOUS_REG, 0, SEEK_SET, buff, 128);
    if((0 == stat(RSA_MODULOUS_REG, &st)) && (128==len) && (buff[0]!=0))
	{
		mod = InPutFromAddr(buff, 128);
		ret = rtk_verifyv2(package_file_path, mod);
		printf("[JH] verify sys:%d\r\n", ret);
		return (0 == ret) ? true:false;
	}
	else if(0 == stat(RSA_MODULOUS_REG_TMP,  &st))
	{
		DumpFileToMemory(RSA_MODULOUS_REG_TMP, 0, SEEK_SET, buff, 256);
		mod = InPutFromStr((char*)buff, HEX);
		ret = rtk_verifyv2(package_file_path, mod);
		printf("[JH] verify tmp:%d\r\n", ret);
		return (0 == ret) ? true:false;
	}
	else
	{
		return rtk_verifyv1(package_file_path);
	}
}

