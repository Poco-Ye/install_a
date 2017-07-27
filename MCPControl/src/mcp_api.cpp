/***************************************************************************************************
  File        : mcp_api.cpp   
  Description : Low Level API for MCP 
  Author      : Kevin Wang 
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------     
    20090605    | Create Phase    
***************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "mcp_config.h"
#include "MCPUtil.h"
#include "ionapi.h"
#include <sys/mman.h>
#include "mcp_api.h"
#include <rtk_shaAPI.h>

static int m_fd = 0;
static int ionfd = -1;

#define MAX_CW_ENTRY         128

#define S_OK 0
#define S_FALSE -1

#define LOG_NDEBUG 	     1
#define MYPRINT printf
#define ROUNDUP(num, align) ((num + align - 1) & ~(align - 1))

void _DumpDesciptor(MCP_DESC* d)
{
    printf("\n\nXXXXXXXX desc (%p) XXXXXXXXXX\n",d);
    printf("flag=%08x\n",d->flags);
    printf("Key0=%08x\n",d->Key[0]);
    printf("Key1=%08x\n",d->Key[1]);
    printf("Key2=%08x\n",d->Key[2]);
    printf("Key3=%08x\n",d->Key[3]);
    printf("Key4=%08x\n",d->Key[4]);
    printf("Key5=%08x\n",d->Key[5]);
    printf("IV0=%08x\n",d->IV[0]);
    printf("IV1=%08x\n",d->IV[1]);
    printf("IV2=%08x\n",d->IV[2]);
    printf("IV3=%08x\n",d->IV[3]);
    printf("di=%08x\n",d->data_in);
    printf("do=%08x\n",d->data_out);
    printf("len=%08x\n",d->length);
    printf("XXXXXXXX desc end XXXXXXXXXX\n\n");
}
// flush the cached virtual address. There is no need to call this function if use non-cached virtual address
void  ion_flushMemory(void* ptr, long size)
{
  return;
}

void RTKIon_free(void *addr, unsigned int size, ion_user_handle_t *ionhdl)
{
    munmap(addr, ROUNDUP(size, getpagesize()));
    ion_free(ionfd, *ionhdl);
}
void* RTKIon_alloc(OSAL_U32 size, OSAL_U8 **noncacheable, OSAL_U32 *phys_addr, ion_user_handle_t *ionhdl)
{
    OSAL_S32 ret;
    OSAL_S32 mapfd;
    OSAL_U32 alloc_size = ROUNDUP(size, getpagesize());

    if(ionfd == -1)
    {
        ionfd = ion_open();
        if(ionfd < 0)
            return NULL;
    }

    if(size < alloc_size)
        MYPRINT("%s: size:%x round up to %x\n", __func__, size, alloc_size);

    ret = ion_alloc(ionfd, alloc_size, getpagesize(), RTK_PHOENIX_ION_HEAP_MEDIA_MASK, (ION_FLAG_NONCACHED | ION_FLAG_SCPUACC | ION_FLAG_HWIPACC), ionhdl);
    //ret = ion_alloc(ionfd, alloc_size, getpagesize(), RTK_PHOENIX_ION_HEAP_MEDIA_MASK, (ION_FLAG_SCPUACC | ION_FLAG_HWIPACC), ionhdl);
    if (ret < 0)
    {
        MYPRINT("In[%s][%d] fail to allocate frame buffer, size:%d", __func__, __LINE__, size);
        return NULL;
    }
    ret = ion_phys(ionfd, *ionhdl, phys_addr, (unsigned int*)&alloc_size);
    if (ret < 0)
    {
        MYPRINT("In[%s][%d] fail to ion_phys", __func__, __LINE__);
        return NULL;
    }
    ret = ion_map(ionfd, *ionhdl, alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED, 0, (OSAL_U8 **)noncacheable, (int*)&mapfd);
    if (ret < 0)
    {
        MYPRINT("In[%s][%d] fail to ion_map", __func__, __LINE__);
        return NULL;
    }
    close(mapfd); //we don't need to share with other process, close it

    MYPRINT("%s: size:%d alloc_size:%d ionhdl:%p phys_addr:%lx noncacheable:%p\n", __func__, size, alloc_size, *ionhdl, *phys_addr, *noncacheable);
    return (void *)*noncacheable;
}

/*====================================================================== 
 * Func : MCP_Init 
 *
 * Desc : Init MCP module 
 *
 * Parm : N/A
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_Init()
{       
    if (!m_fd) 
    {                           
        if ((m_fd = open("/dev/mcp_core", O_RDWR))>=0)                    
            return S_OK;
                            
        if ((m_fd = open("/dev/mcp/core", O_RDWR))>=0)        // new version                                    
            return S_OK;                    
            
        if ((m_fd = open("/dev/mcp", O_RDWR))>=0)                    
            return S_OK;                        
                                         
        printf("MCP Init failed - open file error %d\n", m_fd);        
        return S_FALSE;                     
    }
    
    return S_OK;
}

/*====================================================================== 
 * Func : MCP_UnInit 
 *
 * Desc : Uninit MCP module 
 *
 * Parm : N/A
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
void MCP_UnInit()
{
    if (m_fd) 
    {            
        close(m_fd);
        m_fd = 0;
            
    }
}



/*====================================================================== 
 * Func : MCP_DoCommand 
 *
 * Desc : Do Command
 *
 * Parm : pDesc : Descriptors to be Execute
 *        nDesc : number of Descriptor to be Execute
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_DoCommand(MCP_DESC* pDesc, int nDesc)
{       
    MCP_DESC_SET DescSet = {pDesc, nDesc};
                        
    MCP_Init();
    
    if (m_fd && ioctl(m_fd, MCP_IOCTL_DO_COMMAND, &DescSet)==0)            
        return S_OK;    
        
    printf("Do Command Failed\n");
    return S_FALSE;        
}



/********************************************************************************
 DES
 ********************************************************************************/



/*====================================================================== 
 * Func : MCP_DES_DESC_INIT
 *
 * Desc : Init DES Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *        EnDe       : Encryption/Descryption
 *              0 for Decryption / 1 for Encryption
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_DES_DESC_INIT(
    MCP_DESC*               pDesc, 
    unsigned char           EnDe,
    unsigned char           Mode,
    unsigned char           Key[7],
    unsigned char           IV[8]
    )
{    
    memset(pDesc, 0, sizeof(MCP_DESC));
    
    pDesc->flags = MARS_MCP_MODE(MCP_ALGO_DES) | MARS_MCP_BCM(Mode) | MARS_MCP_ENC(EnDe);
         
    if (Key==NULL)
    {                
        pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_OTP);                          
    }
    else if ((unsigned long)Key <= MAX_CW_ENTRY)     
    {
        pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_CW);
        pDesc->Key[0] = MCP_CW_KEY((unsigned long)Key-1);    
        pDesc->Key[1] = 0;
        pDesc->Key[2] = 0;
        pDesc->Key[3] = 0;        
    }
    else  
    {                        
        pDesc->Key[0]  = (Key[0]<<24) + (Key[1]<<16) + (Key[2]<<8) + Key[3];  
        pDesc->Key[1]  = (Key[4]<<16) + (Key[5]<<8)  +  Key[6];         
    }
        
    if (IV)        
    {
        pDesc->IV[0]   = (IV[0]<<24) + (IV[1]<<16) + (IV[2]<<8) + IV[3];  
        pDesc->IV[1]   = (IV[4]<<24) + (IV[5]<<16) + (IV[6]<<8) + IV[7];  
    }
    
    return S_OK;
}






/*====================================================================== 
 * Func : MCP_DES_Decryption
 *
 * Desc : Do DES Decryption
 *
 * Parm : pDesc      : Descriptor to be Load 
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In
 *        DataOut    : Data Out
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_DES_Decryption(    
    unsigned char           Mode,
    unsigned char           Key[7],
    unsigned char           IV[8],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_DES_DESC_INIT(&Desc, 0, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len;    
    
    return MCP_DoCommand(&Desc, 1);   
}




/*====================================================================== 
 * Func : MCP_DES_Encryption
 *
 * Desc : Do DES Encryption
 *
 * Parm : pDesc      : Descriptor to be Load 
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In
 *        DataOut    : Data Out
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_DES_Encryption(    
    unsigned char           Mode,
    unsigned char           Key[7],
    unsigned char           IV[8],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_DES_DESC_INIT(&Desc, 1, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len;    
    
    return MCP_DoCommand(&Desc, 1);   
}



/********************************************************************************
 TDES 
 ********************************************************************************/


/*====================================================================== 
 * Func : MCP_TDES_DESC_INIT
 *
 * Desc : Init TDES Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *        EnDe       : Encryption/Descryption
 *              0 for Decryption / 1 for Encryption
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_TDES_DESC_INIT(
    MCP_DESC*               pDesc, 
    unsigned char           EnDe,
    unsigned char           Mode,
    unsigned char           Key[21],
    unsigned char           IV[8]
    )
{    
    memset(pDesc, 0, sizeof(MCP_DESC));
    
    pDesc->flags = MARS_MCP_MODE(MCP_ALGO_3DES) | MARS_MCP_BCM(Mode) | MARS_MCP_ENC(EnDe);                        
                    
    if (Key==NULL)
    {                
        pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_OTP);                          
    }
    else if ((unsigned long)Key <= MAX_CW_ENTRY)     
    {
        pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_CW);
        pDesc->Key[0] = MCP_CW_KEY((unsigned long)Key-1);    
        pDesc->Key[1] = 0;
        pDesc->Key[2] = 0;
        pDesc->Key[3] = 0;        
    }
    else        
    {                            
        if (EnDe)                    
        {                 
            // Key A                    
            pDesc->Key[0]  = (Key[ 0]<<24) + (Key[ 1]<<16) + (Key[ 2]<<8) + Key[3];  
            pDesc->Key[1]  = (Key[ 4]<<16) + (Key[ 5]<<8)  +  Key[ 6];               
        
            // Key B
            pDesc->Key[2]  = (Key[ 7]<<24) + (Key[ 8]<<16) + (Key[ 9]<<8) + Key[10];  
            pDesc->Key[3]  = (Key[11]<<16) + (Key[12]<<8)  +  Key[13];               
        
            // Key C
            pDesc->Key[4]  = (Key[14]<<24) + (Key[15]<<16) + (Key[16]<<8) + Key[17];  
            pDesc->Key[5]  = (Key[18]<<16) + (Key[19]<<8)  +  Key[20];           
        }
        else
        {
            // Key C                    
            pDesc->Key[4]  = (Key[ 0]<<24) + (Key[ 1]<<16) + (Key[ 2]<<8) + Key[3];  
            pDesc->Key[5]  = (Key[ 4]<<16) + (Key[ 5]<<8)  +  Key[ 6];               
        
            // Key B
            pDesc->Key[2]  = (Key[ 7]<<24) + (Key[ 8]<<16) + (Key[ 9]<<8) + Key[10];  
            pDesc->Key[3]  = (Key[11]<<16) + (Key[12]<<8)  +  Key[13];               
        
            // Key A
            pDesc->Key[0]  = (Key[14]<<24) + (Key[15]<<16) + (Key[16]<<8) + Key[17];  
            pDesc->Key[1]  = (Key[18]<<16) + (Key[19]<<8)  +  Key[20];           
        }
    }
    
    if (IV)
    {    
        pDesc->IV[0]   = (IV[0]<<24) + (IV[1]<<16) + (IV[2]<<8) + IV[3];  
        pDesc->IV[1]   = (IV[4]<<24) + (IV[5]<<16) + (IV[6]<<8) + IV[7];  
    }
    return S_OK;
}






/*====================================================================== 
 * Func : MCP_TDES_Decryption
 *
 * Desc : Do TDES Decryption
 *
 * Parm : pDesc      : Descriptor to be Load 
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In
 *        DataOut    : Data Out
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_TDES_Decryption(    
    unsigned char           Mode,
    unsigned char           Key[21],
    unsigned char           IV[8],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_TDES_DESC_INIT(&Desc, 0, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len;    
    
    return MCP_DoCommand(&Desc, 1);   
}




/*====================================================================== 
 * Func : MCP_TDES_Encryption
 *
 * Desc : Do TDES Encryption
 *
 * Parm : pDesc      : Descriptor to be Load 
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In
 *        DataOut    : Data Out
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_TDES_Encryption(    
    unsigned char           Mode,
    unsigned char           Key[21],
    unsigned char           IV[8],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_TDES_DESC_INIT(&Desc, 1, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len;    
    
    return MCP_DoCommand(&Desc, 1);   
}




/********************************************************************************
 AES
 ********************************************************************************/



/*====================================================================== 
 * Func : MCP_AES_DESC_INIT
 *
 * Desc : Init AES Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *        EnDe       : Encryption/Descryption
 *              0 for Decryption / 1 for Encryption
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_DESC_INIT(
    MCP_DESC*               pDesc, 
    unsigned char           EnDe,
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16]
    )
{            
    memset(pDesc, 0, sizeof(MCP_DESC));
    
    switch (Mode)
    {
    case MCP_BCM_ECB:        
    case MCP_BCM_CBC:
    case MCP_BCM_CTR:                
        
        pDesc->flags = MARS_MCP_MODE(MCP_ALGO_AES) | MARS_MCP_BCM(Mode) | MARS_MCP_ENC(EnDe);
                
        //printf("***************(unsigned long)Key: %x\n",(unsigned long)Key);
                
        if (Key==NULL)
        {                
            pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_OTP);                          
        }
        else if ((unsigned long)Key <= MAX_CW_ENTRY)     
        {
            pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_CW);
            pDesc->Key[0] = MCP_CW_KEY((unsigned long)Key-1);    
            pDesc->Key[1] = 0;
            pDesc->Key[2] = 0;
            pDesc->Key[3] = 0;        
        }        
        else
        {
            pDesc->Key[0]  = (Key[ 0]<<24) + (Key[ 1]<<16) + (Key[ 2]<<8) + Key[ 3];  
            pDesc->Key[1]  = (Key[ 4]<<24) + (Key[ 5]<<16) + (Key[ 6]<<8) + Key[ 7];  
            pDesc->Key[2]  = (Key[ 8]<<24) + (Key[ 9]<<16) + (Key[10]<<8) + Key[11];  
            pDesc->Key[3]  = (Key[12]<<24) + (Key[13]<<16) + (Key[14]<<8) + Key[15];  
        }            
            
        if (IV)        
        {
            pDesc->IV[0]   = (IV[ 0]<<24) + (IV[ 1]<<16) + (IV[ 2]<<8) + IV[ 3];  
            pDesc->IV[1]   = (IV[ 4]<<24) + (IV[ 5]<<16) + (IV[ 6]<<8) + IV[ 7];  
            pDesc->IV[2]   = (IV[ 8]<<24) + (IV[ 9]<<16) + (IV[10]<<8) + IV[11];  
            pDesc->IV[3]   = (IV[12]<<24) + (IV[13]<<16) + (IV[14]<<8) + IV[15]; 
        }
    
        return S_OK;                
        
    default:        
        printf("[MCP] Init AES descriptor failed - invalid mode (%d)\n", Mode);
        return S_FALSE;
    }        
}


/*====================================================================== 
 * Func : MCP_AES_256_DESC_INIT
 *
 * Desc : Init AES Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *        EnDe       : Encryption/Descryption
 *              0 for Decryption / 1 for Encryption
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_256_DESC_INIT(
    MCP_DESC*               pDesc, 
    unsigned char           EnDe,
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned char         AesMode
    )
{            
    memset(pDesc, 0, sizeof(MCP_DESC));
    
    switch (Mode)
    {
    case MCP_BCM_ECB:        
    case MCP_BCM_CBC:
    case MCP_BCM_CTR:                
        
        pDesc->flags = MARS_MCP_MODE(AesMode) | MARS_MCP_BCM(Mode) | MARS_MCP_ENC(EnDe);
                
        //printf("***************(unsigned long)Key: %x\n",(unsigned long)Key);
                
        if (Key==NULL)
        {                
            pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_OTP);                          
        }
        else //Key must in CW
        {
            pDesc->flags |= MARS_MCP_KEY_SEL(MCP_KEY_SEL_CW);
            pDesc->Key[0] = (unsigned long)Key;    
            pDesc->Key[1] = 0;
            pDesc->Key[2] = 0;
            pDesc->Key[3] = 0;        
        }        
     
            
        if (IV)        
        {
            pDesc->IV[0]   = (IV[ 0]<<24) + (IV[ 1]<<16) + (IV[ 2]<<8) + IV[ 3];  
            pDesc->IV[1]   = (IV[ 4]<<24) + (IV[ 5]<<16) + (IV[ 6]<<8) + IV[ 7];  
            pDesc->IV[2]   = (IV[ 8]<<24) + (IV[ 9]<<16) + (IV[10]<<8) + IV[11];  
            pDesc->IV[3]   = (IV[12]<<24) + (IV[13]<<16) + (IV[14]<<8) + IV[15]; 
        }
    
        return S_OK;                
        
    default:        
        printf("[MCP] Init AES descriptor failed - invalid mode (%d)\n", Mode);
        return S_FALSE;
    }        
}


/*====================================================================== 
 * Func : MCP_AES_Decryption
 *
 * Desc : Do AES Decryption
 *
 * Parm : Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In    (Physical Address)
 *        DataOut    : Data Out   (Physical Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_Decryption(    
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_AES_DESC_INIT(&Desc, 0, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len & ~0xF;    
    
    return MCP_DoCommand(&Desc, 1);   
}




/*====================================================================== 
 * Func : MCP_AES_Encryption
 *
 * Desc : Do AES Encryption
 *
 * Parm : Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In      (Physical Address)
 *        DataOut    : Data Out     (Physical Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_Encryption(    
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len
    )
{    
    MCP_DESC Desc;
    
    MCP_AES_DESC_INIT(&Desc, 1, Mode, Key, IV);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len & ~0xF;
    
    return MCP_DoCommand(&Desc, 1);   
}

/*====================================================================== 
 * Func : MCP_AES_256_Decryption
 *
 * Desc : Do AES Decryption
 *
 * Parm : Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In    (Physical Address)
 *        DataOut    : Data Out   (Physical Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_256_Decryption(    
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len,
    unsigned char         AesMode
    )
{    
    MCP_DESC Desc;
    
    MCP_AES_256_DESC_INIT(&Desc, 0, Mode, Key, IV, AesMode);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len & ~0xF;    
    
    return MCP_DoCommand(&Desc, 1);   
}




/*====================================================================== 
 * Func : MCP_AES_Encryption
 *
 * Desc : Do AES Encryption
 *
 * Parm : Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        DataIn     : Data In      (Physical Address)
 *        DataOut    : Data Out     (Physical Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_256_Encryption(    
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned long           DataIn,
    unsigned long           DataOut,
    unsigned long           Len,
    unsigned char         AesMode
    )
{    
    MCP_DESC Desc;
    
    MCP_AES_256_DESC_INIT(&Desc, 1, Mode, Key, IV, AesMode);
    
    Desc.data_in  = DataIn;
    Desc.data_out = DataOut;
    Desc.length   = Len & ~0xF;
    
    return MCP_DoCommand(&Desc, 1);   
}


/********************************************************************************
 RC4
 ********************************************************************************/



/*====================================================================== 
 * Func : MCP_RC3_DESC_INIT
 *
 * Desc : Init RC4 Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *        EnDe       : Encryption/Descryption
 *              0 for Decryption / 1 for Encryption
 *        Key        : Key Value
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_RC4_DESC_INIT(
    MCP_DESC*               pDesc, 
    unsigned char           EnDe,
    unsigned char           Key[16]
    )
{            
    memset(pDesc, 0, sizeof(MCP_DESC));

    {
        pDesc->flags = MARS_MCP_MODE(MCP_ALGO_RC4) | MARS_MCP_ENC(EnDe);
        printf("%s-%s : pDesc->flags = %08x\n", __FILE__,__func__,pDesc->flags);            
        if (Key)                                        
        {            
            pDesc->Key[0]  = (Key[ 0]<<24) + (Key[ 1]<<16) + (Key[ 2]<<8) + Key[ 3];  
            pDesc->Key[1]  = (Key[ 4]<<24) + (Key[ 5]<<16) + (Key[ 6]<<8) + Key[ 7];  
            pDesc->Key[2]  = (Key[ 8]<<24) + (Key[ 9]<<16) + (Key[10]<<8) + Key[11];  
            pDesc->Key[3]  = (Key[12]<<24) + (Key[13]<<16) + (Key[14]<<8) + Key[15];  
        }
	else
	{
            printf("Init RC4 descriptor failed - key is NULL\n");
            return -1;
        }             
        return 0;                
    }        
}

/*====================================================================== 
 * Func : MCP_RC4_Decryption
 *
 * Desc : Do RC4 Decryption
 *
 * Parm : 
 *        key        : Key Value
 *        p_in       : Data In 
 *        p_out      : Data Out
 *        len        : Data Length
 *
 * Retn : 0 for success, others failed
 *======================================================================*/
int MCP_RC4_Decryption(    
    unsigned char           Key[16],
    unsigned long          p_in,
    unsigned long          p_out,
    unsigned long           len
    )
{    
    MCP_DESC	desc;
    int         ret;    

    MCP_RC4_DESC_INIT(&desc, 0, Key);


    desc.data_in  = p_in;
    desc.data_out = p_out;
    desc.length   = len & ~0xF;    
    
    ret = MCP_DoCommand(&desc, 1);   
    
    return ret;
}




/*====================================================================== 
 * Func : MCP_RC4_Encryption
 *
 * Desc : Do RC4 Encryption
 *
 * Parm : 
 *        key        : Key Value
 *        p_in       : Data In 
 *        p_out      : Data Out
 *        len        : Data Length
 *
 * Retn : 0 for success, others failed
 *======================================================================*/
int MCP_RC4_Encryption(    
    unsigned char           key[16],
    unsigned long          p_in,
    unsigned long          p_out,
    unsigned long           len
    )
{    
    MCP_DESC	desc;
    int         ret;    

    MCP_RC4_DESC_INIT(&desc, 1, key);

    desc.data_in  = p_in;
    desc.data_out = p_out;
    desc.length   = len & ~0xF;

    ret = MCP_DoCommand(&desc, 1);   

    return ret; 
}

/*------------------------------------------------------------------ 
 * Func : MCP_RC4_128_DataEncryptTest
 *
 * Desc : Test RC4 128 
 *
 * Parm : N/A
 *
 * Retn : N/A
 *------------------------------------------------------------------*/
void MCP_RC4_128_DataEncryptTest(void)
{
    unsigned char Data[16]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 
                            0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};        
    unsigned char Key[16]; 
    
    memset(Key, 'b', sizeof(Key));

    MCP_RC4_Encryption(Key, (unsigned long)Data, (unsigned long)Data, 16);

    //mcp_dump_data_with_text(Data, 16, "encrypted value : ");    

    MCP_RC4_Decryption(Key, (unsigned long)Data, (unsigned long)Data, 16);

    //mcp_dump_data_with_text(Data, 16, "decrypted value : ");    
}

/********************************************************************************
 AES Hash
 ********************************************************************************/




/*====================================================================== 
 * Func : MCP_AES_H_HASH_INIT
 *
 * Desc : Init AES Hashing Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_H_HASH_INIT(MCP_DESC* pDesc)
{
    memset(pDesc, 0, sizeof(MCP_DESC));
    
    pDesc->flags = MARS_MCP_MODE(MCP_ALGO_AES_H) | MARS_MCP_BCM(MCP_BCM_ECB) | MARS_MCP_ENC(1);                        
                        
    pDesc->IV[0]   = 0x2DC2DF39;  
    pDesc->IV[1]   = 0x420321D0;
    pDesc->IV[2]   = 0xCEF1FE23;
    pDesc->IV[3]   = 0x74029D95;    
    return S_OK;
}



/*====================================================================== 
 * Func : MCP_AES_H_IV_UPDATE
 *
 * Desc : Update Initial Vector of AES Hashing Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load 
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_H_IV_UPDATE(MCP_DESC* pDesc, unsigned char Hash[16])
{
    pDesc->IV[0]   = (Hash[ 0]<<24) + (Hash[ 1]<<16) + (Hash[ 2]<<8) + Hash[ 3]; 
    pDesc->IV[1]   = (Hash[ 4]<<24) + (Hash[ 5]<<16) + (Hash[ 6]<<8) + Hash[ 7];
    pDesc->IV[2]   = (Hash[ 8]<<24) + (Hash[ 9]<<16) + (Hash[10]<<8) + Hash[11];
    pDesc->IV[3]   = (Hash[12]<<24) + (Hash[13]<<16) + (Hash[14]<<8) + Hash[15];  
    return S_OK;
}



/*------------------------------------------------------------------ 
 * Func : MCP_AES_H_PADDING
 *
 * Desc : padding tail to the end of the last data block. 
 *
 * Parm : p_buff        : data buffer 
 *        buff_len      : length of buffer
 *        data_len      : total length of data in byte(from 1st block to 
 *                        the last block)
 * 
 * Retn : > 0 : number of bytes padded
 *------------------------------------------------------------------*/
int MCP_AES_H_PADDING(    
    unsigned char*          p_buff,
    unsigned long           buff_len,
    unsigned long           data_len
    )
{    
//#if (IS_CHIP(MARS))
#if 1
    // don't need to padding data, mars has auto data padding
    return 0;
#else

    unsigned long long bit_len = ((unsigned long long) data_len) << 3; 
    int pad_sz = 16 - (data_len & 0xF);    
    
    if (pad_sz <= 8)
        pad_sz += 16;
    
    if (buff_len < pad_sz)
    {
        printf("[MCP] Warning, padding AES_H data failed, no enough space left for data padding. pad_sz = %d, remain_sz = %ld\n", pad_sz, buff_len);
        return 0;
    }
        
    memset(p_buff, 0, pad_sz);
        
    *p_buff |= 0x80;
    
    // pad bit length in big endian format
    p_buff += pad_sz-8;
    *p_buff++ = (bit_len >> 56) & 0xFF;
    *p_buff++ = (bit_len >> 48) & 0xFF;
    *p_buff++ = (bit_len >> 40) & 0xFF;
    *p_buff++ = (bit_len >> 32) & 0xFF;
    *p_buff++ = (bit_len >> 24) & 0xFF;
    *p_buff++ = (bit_len >> 16) & 0xFF;
    *p_buff++ = (bit_len >>  8) & 0xFF;
    *p_buff++ = (bit_len)       & 0xFF;           

    return pad_sz;  
#endif    
}




/*====================================================================== 
 * Func : MCP_AES_H_Hashing
 *
 * Desc : Do AES H Hashing
 *
 * Parm : pDesc      : Descriptor to be Load 
 *        DataIn     : Data to be hashing (in Physical Address) 
 *        Len        : number of data to be hashing 
 *        DataOut    : Physical address that used to store hash value
 *
 * Retn : S_OK  /  S_FALSE 
 *======================================================================*/
int MCP_AES_H_Hashing(
    MCP_DESC*               pDesc, 
    unsigned long           DataIn,     
    unsigned long           Len,
    unsigned long           DataOut
    )
{        
    pDesc->data_in  = DataIn;
    pDesc->data_out = DataOut;
    pDesc->length   = Len;                                
                
    return MCP_DoCommand(pDesc, 1);
}


/////////////////////////////////// Misc Functions ////////////////////////////////////
/*====================================================================== 
 * Func : MCP_RC4_DataDecryption
 *
 * Desc : Do RC4 Decryption (for data virtual memory)
 *
 * Parm : 
 *        Key        : Key Value
 *        pDataIn    : Data In    (Virtual Address)
 *        pDataOut   : Data Out   (Virtual Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_RC4_DataDecryption(    
    unsigned char           Key[16],
    unsigned char*          pDataIn,
    unsigned char*          pDataOut,
    unsigned long           Len
    )
{           
    int ret;                
    unsigned char*  pBuff = NULL;
    unsigned char*  pBuffVirt = NULL;
    unsigned long   PhyAddr;            
    ion_user_handle_t ionhdl_headr;

    pBuff = (unsigned char *)RTKIon_alloc(Len, &pBuffVirt, &PhyAddr, &ionhdl_headr);
    
    if (!pBuff)
    {
        printf("%s : %d : %s    FATAL : allocate rc4 decrypt ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                
        return S_FALSE;
    }        
            
    memcpy(pBuffVirt, pDataIn, Len);
    
    ion_flushMemory(pBuffVirt, Len);
    
    ret = MCP_RC4_Decryption(Key, PhyAddr, PhyAddr, Len);    
          
    memcpy(pDataOut, pBuffVirt, Len);
    
    if(pBuff) {
	MYPRINT("Release rc4 decrypt ion memory\n");
	RTKIon_free(pBuff, Len, &ionhdl_headr);
    }
    return ret;      
}
/*====================================================================== 
 * Func : MCP_RC4_DataEncryption
 *
 * Desc : Do RC4 Encryption (for data virtual memory)
 *
 * Parm : 
 *        Key        : Key Value
 *        pDataIn    : Data In    (Virtual Address)
 *        pDataOut   : Data Out   (Virtual Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_RC4_DataEncryption(    
    unsigned char           Key[16],
    unsigned char*          pDataIn,
    unsigned char*          pDataOut,
    unsigned long           Len
    )
{           
    int ret;                
    unsigned char*  pBuff = NULL;
    unsigned char*  pBuffVirt = NULL;
    unsigned long   PhyAddr;            
    ion_user_handle_t ionhdl_headr;

    pBuff = (unsigned char *)RTKIon_alloc(Len, &pBuffVirt, &PhyAddr, &ionhdl_headr);
    
    if (!pBuff)
    {
        printf("%s : %d : %s    FATAL : allocate rc4 decrypt ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                
        return S_FALSE;
    }        
            
    memcpy(pBuffVirt, pDataIn, Len);
    
    ion_flushMemory(pBuffVirt, Len);
    
    ret = MCP_RC4_Encryption(Key, PhyAddr, PhyAddr, Len);    
          
    memcpy(pDataOut, pBuffVirt, Len);
    
    if(pBuff) {
	MYPRINT("Release rc4 decrypt ion memory\n");
	RTKIon_free(pBuff, Len, &ionhdl_headr);
    }
    return ret;      
}
/*====================================================================== 
 * Func : MCP_AES_DataEncryption
 *
 * Desc : Do AES Encryption/Decryption (for data virtual memory)
 *
 * Parm : EnDe       : 0 : Decryption, others : Encryption
 *        Mode       : Operation Mode
 *        Key        : Key Value
 *        IV         : Initial Vector 
 *        pDataIn    : Data In    (Virtual Address)
 *        pDataOut   : Data Out   (Virtual Address)
 *        Len        : Data Length
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_AES_DataEncryption(    
    unsigned char           EnDe,
    unsigned char           Mode,
    unsigned char           Key[16],
    unsigned char           IV[16],
    unsigned char*          pDataIn,
    unsigned char*          pDataOut,
    unsigned long           Len
    )
{           
    int ret;                
    unsigned char*  pBuff = NULL;
    unsigned char*  pBuffVirt = NULL;
    unsigned long   PhyAddr;            
    ion_user_handle_t ionhdl_headr;

    pBuff = (unsigned char *)RTKIon_alloc(Len, &pBuffVirt, &PhyAddr, &ionhdl_headr);
    
    if (!pBuff)
    {
        printf("%s : %d : %s    FATAL : Compute AES_H hash value failed, allocate ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                
        return S_FALSE;
    }        
            
    memcpy(pBuffVirt, pDataIn, Len);
    
    ion_flushMemory(pBuffVirt, Len);
    
    ret = (EnDe) ? MCP_AES_Encryption(Mode, Key, IV, PhyAddr, PhyAddr, Len) :    
                   MCP_AES_Decryption(Mode, Key, IV, PhyAddr, PhyAddr, Len) ;    
          
    memcpy(pDataOut, pBuffVirt, Len);
    
    if(pBuff) {
	MYPRINT("Release the memory\n");
	RTKIon_free(pBuff, Len, &ionhdl_headr);
    }
    return ret;      
}

/*====================================================================== 
 * Func : MCP_AES_H_FileHash
 *
 * Desc : Compute hash value of a file (or part of file) via 128 bits AES_H_Hasing
 *
 * Parm : FName      : File In
 *        Hash[16]   : 128 bits hash value of the file 
 *        BlockSize  : Block Size that used to compute Hashing Value
 *        Start      : Start Address
 *        End        : End   Address
 *
 * Retn : S_OK  /  S_FALSE 
 *======================================================================*/
int MCP_AES_H_FileHash(
    const char*             FName,
    unsigned char           Hash[16],
    unsigned long           BlockSize,
    unsigned long           Start,
    unsigned long           End
    )
{       
    MCP_DESC        Desc;
    FILE*           fd;    
    unsigned char*  pBuff;
    unsigned char*  pBuffVirt;
    unsigned long   PhyAddr;        
    unsigned char*  pHash;
    unsigned long   pHashPhy;    
    int             FileLength;    
    unsigned long   DataLen;
    unsigned long   TotalDataLen = 0;    
    ion_user_handle_t ionhdl_headr;
    
    if (Start >= End)
    {
        printf("%s : %d : %s    FATAL : start address (%d) should less than stop address (%d)\n",
                __FILE__, __LINE__, __FUNCTION__, Start, End);        
        goto error_invalid_range_setting;
    }
         
    if ((fd = fopen(FName, "rb"))==NULL)    
    {
        printf("%s : %d : %s    FATAL : Compute AES_H hash value failed, open file %s failed\n",
                __FILE__, __LINE__, __FUNCTION__, FName);
        goto error_open_file_failed;
    }                           
            
    fseek(fd, 0, SEEK_END);            
        
    FileLength = ftell(fd);
    
    if (End==FILE_END)
    {
        End = FileLength;
    }
    else
    {        
        if (End > FileLength)
        {
            printf("%s : %d : %s    FATAL : stop address (%d) should not larger than file length (%d)\n",
                    __FILE__, __LINE__, __FUNCTION__, End, FileLength);        
            goto error_end_address_out_of_bound;
        }
    }   
    
    fseek(fd, Start, SEEK_SET);                  
        
    pBuff = (unsigned char *)RTKIon_alloc(BlockSize + 16, &pBuffVirt, &PhyAddr, &ionhdl_headr);
    
    if (!pBuff)
    {
        printf("%s : %d : %s    FATAL : Compute AES_H hash value failed, allocate ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                
        goto error_alloc_ion_buffer_failed;
    }
    
    pHash    = pBuffVirt + BlockSize;
    pHashPhy = PhyAddr   + BlockSize;

    MCP_AES_H_HASH_INIT(&Desc);
    
    DataLen = End - Start;
    TotalDataLen =  DataLen;      
         
    while(DataLen)
    {
        int Len = fread(pBuff, 1, (DataLen >=BlockSize) ? BlockSize : DataLen, fd);
                                  
        if (Len > 0)
        {                                                     
            DataLen -= Len;
                        
            if (DataLen==0)            
                Len += MCP_AES_H_PADDING(&pBuff[Len], BlockSize-Len, TotalDataLen);

            ion_flushMemory(pBuff, Len);
            
            ion_flushMemory(pHash, 16);
            
            MCP_AES_H_Hashing(&Desc, PhyAddr, Len, pHashPhy);                        
            
            MCP_AES_H_IV_UPDATE(&Desc, pHash);  // Update Hash Value            
            
            //printf("[%08x]hash value = %08x %08x %08x %08x\n", total_len, Desc.IV[0], Desc.IV[1], Desc.IV[2], Desc.IV[3]  );              
        }
        else 
        {
            printf("%s:%d:%s Compute file hash value failed - %s\n", 
                    __FILE__, __LINE__ ,__FUNCTION__, strerror(errno));          
            goto error_read_file_failed;                      
        }            
    }
                  
    memcpy(Hash, pHash, 16);
    
    if(pBuff) {
	MYPRINT("Release the memory\n");
	RTKIon_free(pBuff, BlockSize + 16, &ionhdl_headr);
    }
            
    fclose(fd);      
        
    return S_OK;
    
//----
error_read_file_failed:
    if(pBuff) {
	MYPRINT("Release the memory\n");
	RTKIon_free(pBuff, BlockSize + 16, &ionhdl_headr);
    }
error_alloc_ion_buffer_failed:
error_end_address_out_of_bound:
    fclose(fd);  
   
error_open_file_failed:
error_invalid_range_setting:
    return S_FALSE;        
}

/*====================================================================== 
 * Func : MCP_AES_H_DataHashEx
 *
 * Desc : Compute hash value of Data via 128 bits AES_H_Hasing
 *
 * Parm : pData      : Start address of data
 *        DataLen    : Length of data
 *        Hash[16]   : 128 bits hash value of the file 
 *        BlockSize  : Block Size that used to compute Hashing Value 
 *        IsFirstBlock : Is the data is the 1st block    
 *
 * Retn : S_OK  /  S_FALSE 
 *======================================================================*/
int MCP_AES_H_DataHashEx(
    unsigned char*          pData,
    unsigned long           DataLen,
    unsigned char           Hash[16],
    unsigned long           BlockSize,
    unsigned char           IsFirstBlock
    )
{       
    MCP_DESC        Desc;      
    unsigned char*  pBuff;
    unsigned char*  pBuffVirt;
    unsigned long   PhyAddr;        
    unsigned char*  pHash;
    unsigned long   pHashPhy;        
    ion_user_handle_t ionhdl_headr;
    
    pBuff = (unsigned char *)RTKIon_alloc(BlockSize + 16, &pBuffVirt, &PhyAddr, &ionhdl_headr);
    
    if (!pBuff)
    {
        printf("%s : %d : %s    FATAL : Compute AES_H hash value failed, allocate ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                
        return S_FALSE;
    }
    
    pHash    = pBuffVirt + BlockSize;
    pHashPhy = PhyAddr   + BlockSize;

    MCP_AES_H_HASH_INIT(&Desc);        
    
    if (!IsFirstBlock)
        MCP_AES_H_IV_UPDATE(&Desc, Hash);
                    
    while(DataLen)
    {
        unsigned long Len = (DataLen > BlockSize) ? BlockSize : DataLen;
        
        memcpy(pBuff, pData, Len);
                                                                 
        DataLen -= Len;
            
        ion_flushMemory(pBuff, Len);
            
        ion_flushMemory(pHash, 16);
            
        MCP_AES_H_Hashing(&Desc, PhyAddr, Len, pHashPhy);                        
            
        MCP_AES_H_IV_UPDATE(&Desc, pHash);  // Update Hash Value            
            
        //printf("[%08x]hash value = %08x %08x %08x %08x\n", total_len, Desc.IV[0], Desc.IV[1], Desc.IV[2], Desc.IV[3]  );                      
    }
          
    memcpy(Hash, pHash, 16);
    
    if(pBuff) {
	MYPRINT("Release the memory\n");
	RTKIon_free(pBuff, BlockSize + 16, &ionhdl_headr);
    }
        
    return S_OK;    
}

/*====================================================================== 
 * Func : MCP_SHA1_DESC_INIT
 *
 * Desc : Init SHA1 Descriptor
 *
 * Parm : pDesc      : Descriptor to be Load
 *
 * Retn : S_OK /  S_FALSE 
 *======================================================================*/
int MCP_SHA1_DESC_INIT(
    MCP_DESC*               pDesc
    )
{            
    memset(pDesc, 0, sizeof(MCP_DESC));

    pDesc->flags = 0x0004;
        printf ("[Installer_D]: Use default IV key.\n");
        pDesc->Key[0] = SHA1_IV_0;
        pDesc->Key[1] = SHA1_IV_1;
        pDesc->Key[2] = SHA1_IV_2;
        pDesc->Key[3] = SHA1_IV_3;
        pDesc->Key[4] = SHA1_IV_4;
    return S_OK;                
}


int SHA1_hash(unsigned int pDataIn, int length, unsigned int pDataOut, unsigned int iv[5])
{
    MCP_DESC Desc;

    int ret;
#if 0
    unsigned char*  pBuff_src = NULL;
    unsigned char*  pBuffVirt_src = NULL;
    unsigned long   PhyAddr_src;            
    ion_user_handle_t ionhdl_headr_src;

    unsigned char*  pBuff_dst = NULL;
    unsigned char*  pBuffVirt_dst = NULL;
    unsigned long   PhyAddr_dst;            
    ion_user_handle_t ionhdl_headr_dst;

    if ((pDataIn == NULL) || (pDataOut == NULL))
	    return -1;
    
    pBuff_src = (unsigned char *)RTKIon_alloc(length, &pBuffVirt_src, &PhyAddr_src, &ionhdl_headr_src);
    pBuff_dst = (unsigned char *)RTKIon_alloc(length, &pBuffVirt_dst, &PhyAddr_dst, &ionhdl_headr_dst);
        
    if (!pBuff_src || !pBuff_dst)
    {
        printf("%s : %d : %s    FATAL : Compute AES_H hash value failed, allocate ion buffer failed\n",
                __FILE__, __LINE__, __FUNCTION__);
                    
        return S_FALSE;
    }        
                
    memcpy(pBuffVirt_src, pDataIn, length);
        
    ion_flushMemory(pBuffVirt_src, length);
    
    printf("PhyAddr_src: 0x%lx\n", PhyAddr_src);
    printf("PhyAddr_dst: 0x%lx\n", PhyAddr_dst);
#endif
    MCP_SHA1_DESC_INIT(&Desc);
    
    Desc.data_in  = pDataIn;
    Desc.data_out = pDataOut;
    Desc.length   = length & ~0x3F;    

    ret = MCP_DoCommand(&Desc, 1);
 #if 0             
    memcpy(pDataOut, pBuffVirt_dst, length);
        
    if(pBuff_src || pBuff_dst) {
        MYPRINT("Release the memory\n");
        RTKIon_free(pBuff_src, length, &ionhdl_headr_src);
        RTKIon_free(pBuff_dst, length, &ionhdl_headr_dst);
    }
#endif    
    return ret;
   
}

int MCP_SHA256_DESC_INIT(
    MCP_DESC*               pDesc
    )
{            
    memset(pDesc, 0, sizeof(MCP_DESC));

    pDesc->flags = 0xb;
    
    printf ("[MCP_SHA256_DESC_INIT]: Use default IV key.\n");     
    pDesc->Key[0] = SHA256_H0;
    pDesc->Key[1] = SHA256_H1;
    pDesc->Key[2] = SHA256_H2;
    pDesc->Key[3] = SHA256_H3;
    pDesc->Key[4] = SHA256_H4;
    pDesc->Key[5] = SHA256_H5;
    pDesc->IV[0] = SHA256_H6;
    pDesc->IV[1] = SHA256_H7;

    return S_OK;                
}


int SHA256_hash(unsigned int pDataIn, int length, unsigned int pDataOut, unsigned int iv[8])
{
    MCP_DESC Desc;

    int ret;

    MCP_SHA256_DESC_INIT(&Desc);
    
    Desc.data_in  = pDataIn;
    Desc.data_out = pDataOut;
    Desc.length   = length;    

    ret = MCP_DoCommand(&Desc, 1);

    return ret;  
}

