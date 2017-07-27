/*
 * Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <err.h>
#include <tee_client_api.h>
#include <ta_obfuse.h>
#include <stdlib.h>
#include <string.h>

struct keys {
    uint8_t key_seed[16];
    uint8_t key[16];
    uint8_t key1[16];
    uint8_t key2[16];
    uint8_t key3[16];
    //uint8_t rsa_key[256];
};

uint32_t ca_get_secure_bit(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OBFUSE_UUID;
    uint32_t err_origin;
    uint32_t is_secure = 0;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
            TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
                res, err_origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
                        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 0;
    //printf("op.params[0].value.a: 0x%x \n", op.params[0].value.a);

    res = TEEC_InvokeCommand(&sess, TA_CMD_GET_SECURE_BIT, &op,
            &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
                res, err_origin);
    is_secure = op.params[0].value.a;
    //printf("after, op.params[0].value.a: 0x%x \n", op.params[0].value.a);

    TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);

    return is_secure;
}


uint32_t ca_efuse_programmer(
        uint8_t key_seed[16],
        uint8_t key[16],
        uint8_t key1[16],
        uint8_t key2[16],
        uint8_t key3[16],
        uint8_t rsa_key[256])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OBFUSE_UUID;
    uint32_t err_origin;
    uint32_t result;

    struct keys ca_aes_keys;
    uint8_t ca_rsa_key[256];
    
    memset(&ca_aes_keys, 0, sizeof(ca_aes_keys));
    memcpy(&(ca_aes_keys.key_seed), key_seed, 16);
    memcpy(&(ca_aes_keys.key), key, 16);
    memcpy(&(ca_aes_keys.key1), key1, 16);
    memcpy(&(ca_aes_keys.key2), key2, 16);
    memcpy(&(ca_aes_keys.key3), key3, 16);
    memcpy(&(ca_rsa_key), rsa_key, 256);

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT,
					    TEEC_VALUE_INOUT, TEEC_NONE);
    op.params[0].tmpref.buffer = (void *)&ca_aes_keys;
    op.params[0].tmpref.size = sizeof(ca_aes_keys);
    op.params[1].tmpref.buffer = (void *)&ca_rsa_key;
    op.params[1].tmpref.size = sizeof(ca_rsa_key);
    op.params[2].value.a = 0;

    res = TEEC_InvokeCommand(&sess, TA_CMD_KEY_BURN, &op,
                 &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    result = op.params[2].value.a;

    TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);

    return result;
}

uint32_t ca_key_verify(uint8_t *data_in, size_t size)
{
    TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_OBFUSE_UUID;
	uint32_t err_origin;
    uint32_t result;

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INOUT,
					    TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = (void *)data_in;
    op.params[0].tmpref.size = size;
    op.params[1].value.a = 0;

    //printf("op.params[0].tmpref.buffer: 0x%x\n", op.params[0].tmpref.buffer);
    //printf("op.params[0].tmpref.size: 0x%x\n", op.params[0].tmpref.size);

    res = TEEC_InvokeCommand(&sess, TA_CMD_KEY_VERIFY, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
    result = op.params[1].value.a;

    TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

    //printf("result: 0x%x \n", result);

    return result;
}

uint32_t ca_set_tee_os_version(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OBFUSE_UUID;
    uint32_t err_origin;
    uint32_t result = 0;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
                        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 0;

    res = TEEC_InvokeCommand(&sess, TA_CMD_SET_TEE_OS_VERSION, &op,
                 &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);

    result = op.params[0].value.a;

    TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);

    return result;
}

uint32_t ca_get_tee_os_version(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OBFUSE_UUID;
    uint32_t err_origin;
    uint32_t tee_os_version = 0;

    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, err_origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
                        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 0;

    res = TEEC_InvokeCommand(&sess, TA_CMD_GET_TEE_OS_VERSION, &op,
                 &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);

    tee_os_version = op.params[0].value.a;

    TEEC_CloseSession(&sess);

    TEEC_FinalizeContext(&ctx);

    return tee_os_version;
}
