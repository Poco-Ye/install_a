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
#ifndef TA_WRITE_SECURE_STORE_H
#define TA_WRITE_SECURE_STORE_H

/* This UUID is generated with uuidgen
   the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html */
#define INSTALL_AGENT_UUID { 0x58de24e9, 0x0e49, 0x4099, \
		{ 0x9d, 0xd4, 0x65, 0x6d, 0x8d ,0x56, 0x2a, 0x71} }


/* The TAFs ID implemented in this TA */
enum CMD_FOR_TA
{
	TA_CMD_CHECK_PLAYREADY_VERSION = 0x1,
	TA_CMD_UPDATE_PLAYREADY_VERSION = 0x2,
};

#define TEE_FW_VERSION_SIZE 4

//	#define AES_BLOCK_SIZE 16
//	#define INFO_STRING_TITLE 64
//	#define INFO_STRING_SIZE 64
//	#define INFO_FILE_SIZE (INFO_STRING_SIZE*3 + INFO_STRING_TITLE*3 + 6)

#endif /*TA_WRITE_SECURE_STORE_H*/
