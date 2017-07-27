#ifndef __RTK_SHAAPI_H__
#define __RTK_SHAAPI_H__

#ifdef __cplusplus
extern "C" {
#endif
/* Types compatible with OMX types */
typedef unsigned long OSAL_U32;
typedef unsigned char OSAL_U8;
typedef long OSAL_S32;
typedef char OSAL_S8;
typedef void * OSAL_PTR;
typedef unsigned long OSAL_ERRORTYPE;

typedef int ion_user_handle_t;

typedef enum OSAL_BOOL {
    OSAL_FALSE = 0,
    OSAL_TRUE = !OSAL_FALSE,
    OSAL_BOOL_MAX = 0x7FFFFFFF
} OSAL_BOOL;

#define SHA256_SIZE	32

int SHA1_hash(unsigned int pDataIn, int length, unsigned int pDataOut, unsigned int iv[5]);
int SHA256_hash(unsigned int pDataIn, int length, unsigned int pDataOut, unsigned int iv[8]);

void* RTKIon_alloc(OSAL_U32 size, OSAL_U8 **noncacheable, OSAL_U32 *phys_addr, ion_user_handle_t *ionhdl);
void RTKIon_free(void *addr, unsigned int size, ion_user_handle_t *ionhdl);


#ifdef __cplusplus
}
#endif

#endif  // __RTK_SHAAPI_H__

