/*
 * Generic kernel crypto API user space interface library
 *
 * Copyright (C) 2014 - 2016, Stephan Mueller <smueller@chronox.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL2
 * are required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/eventfd.h>
#include <time.h>
#include "cryptouser.h"

#include "kcapi.h"

#define NETLINK_CRYPTO 21 

/* remove once in if_alg.h */
#ifndef ALG_SET_AEAD_ASSOCLEN
#define ALG_SET_AEAD_ASSOCLEN		4
#endif
#ifndef ALG_SET_AEAD_AUTHSIZE
#define ALG_SET_AEAD_AUTHSIZE		5
#endif
#ifndef ALG_SET_PUBKEY
#define ALG_SET_PUBKEY			6
#endif

#ifndef ALG_OP_SIGN
#define ALG_OP_SIGN			2
#endif
#ifndef ALG_OP_VERIFY
#define ALG_OP_VERIFY			3
#endif

/* remove once in socket.h */
#ifndef AF_ALG
#define AF_ALG 38
#endif
#ifndef SOL_ALG
#define SOL_ALG 279
#endif

/* make sure that is equal to include/crypto/if_alg.h */
#ifndef ALG_MAX_PAGES
#define ALG_MAX_PAGES 16
#endif

/************************************************************
 * Declarations for opague data structures
 ************************************************************/

/**
 * Information obtained for different ciphers during handle init time
 * using the NETLINK_CRYPTO interface.
 * @blocksize block size of cipher (hash, symmetric, AEAD)
 * @ivsize size of IV of cipher (symmetric, AEAD)
 * @hash_digestsize size of message digest (hash)
 * @blk_min_keysize minimum key size (symmetric)
 * @blk_max_keysize maximum key size (symmetric)
 * @aead_maxauthsize maximum authentication tag size (AEAD)
 * @rng_seedsize seed size (RNG)
 */
struct kcapi_cipher_info {
	/* generic */
	uint32_t blocksize;
	uint32_t ivsize;
	/* hash */
	uint32_t hash_digestsize;
	/* blkcipher */
	uint32_t blk_min_keysize;
	uint32_t blk_max_keysize;
	/* aead */
	uint32_t aead_maxauthsize;
	/* rng */
	uint32_t rng_seedsize;
};

/**
 * Common data required for symmetric and AEAD ciphers
 * @iv: IV with length of kcapi_cipher_info->ivsize - input
 */
struct kcapi_cipher_data {
	const uint8_t *iv;
};

/**
 * AEAD data
 * @datalen: Length of plaintext / ciphertext data - input
 * @data: Pointer to plaintext / ciphertext data - input / output (the length is
 *	  calculated with: kcapi_skcipher_data->inlen -
 *			   kcapi_aead_data->taglen - kcapi_aead_data->assoclen)
 * @assoclen: Length of associated data - input
 * @assoc: Pointer to associated data - input
 * @taglen: Length of authentication tag - input
 * @tag: Authentication tag - input for decryption, output for encryption
 * @retlen: internal data -- number plaintext / ciphertext bytes returned by
 *	    the read system call
 */
struct kcapi_aead_data {
	uint32_t datalen;
	uint8_t *data;
	uint32_t assoclen;
	uint8_t *assoc;
	uint32_t taglen;
	uint8_t *tag;
};

/**
 * Cipher handle
 * @tfmfd: Socket descriptor for AF_ALG
 * @opfd: FD to open kernel crypto API TFM
 * @pipes: vmplice/splice pipe pair
 * @processed_sg: number of scatter/gather entries sent to the kernel
 * @ciper: Common data for all ciphers
 * @aead: AEAD cipher specific data
 * @info: properties of ciphers
 * @aio: AIO information
 */
struct kcapi_handle {
	int tfmfd;
	int opfd;
	int pipes[2];
	uint32_t processed_sg;
	struct kcapi_cipher_data cipher;
	struct kcapi_aead_data aead;
	struct kcapi_cipher_info info;
	struct kcapi_aio aio;
};

/************************************************************
 * Logging logic
 ************************************************************/
static int kcapi_verbosity_level = LOG_ERR;

static void kcapi_dolog(int severity, const char *fmt, ...)
{
	va_list args;
	char msg[128];
	char sev[16];

	if (severity > kcapi_verbosity_level)
		return;

	memset(sev, 0, sizeof(sev));
	memset(msg, 0, sizeof(msg));

	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg) - 1, fmt, args);
	va_end(args);

	switch (severity) {
	case LOG_DEBUG:
		snprintf(sev, sizeof(sev), "Debug");
		break;
	case LOG_VERBOSE:
		snprintf(sev, sizeof(sev), "Verbose");
		break;
	case LOG_WARN:
		snprintf(sev, sizeof(sev), "Warning");
		break;
	case LOG_ERR:
		snprintf(sev, sizeof(sev), "Error");
		break;
	default:
		snprintf(sev, sizeof(sev), "Unknown");
	}
	fprintf(stderr, "libkcapi - %s: %s\n", sev, msg);
}

void kcapi_set_verbosity(enum kcapi_verbosity level)
{
	kcapi_verbosity_level = level;
}

/************************************************************
 * Internal logic
 ************************************************************/

static int _kcapi_common_accept(struct kcapi_handle *handle)
{
	if (handle->opfd != -1)
		return 0;

	handle->opfd = accept(handle->tfmfd, NULL, 0);
	if (handle->opfd == -1) {
		int errsv = 0;

		errsv = errno;
		kcapi_dolog(LOG_ERR, "AF_ALG: accept failed");
		close(handle->tfmfd);
		handle->tfmfd = -1;
		return -errsv;
	}
	kcapi_dolog(LOG_DEBUG, "AF_ALG: accept syscall successful");

	return 0;
}

static int32_t _kcapi_common_send_meta(struct kcapi_handle *handle,
				       struct iovec *iov, uint32_t iovlen,
				       uint32_t enc, uint32_t flags)
{
	int32_t ret = -EINVAL;
	char *buffer = NULL;
	int errsv = 0;

	/* plaintext / ciphertext data */
	struct cmsghdr *header = NULL;
	uint32_t *type = NULL;
	struct msghdr msg;

	/* IV data */
	struct af_alg_iv *alg_iv = NULL;
	uint32_t iv_msg_size = handle->cipher.iv ?
			  CMSG_SPACE(sizeof(*alg_iv) + handle->info.ivsize) :
			  0;

	/* AEAD data */
	uint32_t *assoclen = NULL;
	uint32_t assoc_msg_size = handle->aead.assoclen ?
				CMSG_SPACE(sizeof(*assoclen)) : 0;

	uint32_t bufferlen =
		CMSG_SPACE(sizeof(*type)) + 	/* Encryption / Decryption */
		iv_msg_size +			/* IV */
		assoc_msg_size;			/* AEAD associated data size */

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	memset(&msg, 0, sizeof(msg));

	buffer = calloc(1, bufferlen);
	if (!buffer)
		return -ENOMEM;

	msg.msg_control = buffer;
	msg.msg_controllen = bufferlen;
	msg.msg_iov = iov;
	msg.msg_iovlen = iovlen;

	/* encrypt/decrypt operation */
	header = CMSG_FIRSTHDR(&msg);
	header->cmsg_level = SOL_ALG;
	header->cmsg_type = ALG_SET_OP;
	header->cmsg_len = CMSG_LEN(sizeof(*type));
	type = (void*)CMSG_DATA(header);
	*type = enc;

	/* set IV */
	if (handle->cipher.iv) {
		header = CMSG_NXTHDR(&msg, header);
		header->cmsg_level = SOL_ALG;
		header->cmsg_type = ALG_SET_IV;
		header->cmsg_len = iv_msg_size;
		alg_iv = (void*)CMSG_DATA(header);
		alg_iv->ivlen = handle->info.ivsize;
		memcpy(alg_iv->iv, handle->cipher.iv, handle->info.ivsize);
	}

	/* set AEAD information */
	if (handle->aead.assoclen) {
		/* Set associated data length */
		header = CMSG_NXTHDR(&msg, header);
		header->cmsg_level = SOL_ALG;
		header->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
		header->cmsg_len = CMSG_LEN(sizeof(*assoclen));
		assoclen = (void*)CMSG_DATA(header);
		*assoclen = handle->aead.assoclen;
	}

	ret = sendmsg(handle->opfd, &msg, flags);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: sendmsg syscall returned %d (errno: %d)",
		    ret, errsv);

	kcapi_memset_secure(buffer, 0, bufferlen);
	free(buffer);
	return (ret >= 0) ? ret : -errsv;
}

static inline int32_t _kcapi_common_send_data(struct kcapi_handle *handle,
					      struct iovec *iov,
					      uint32_t iovlen, uint32_t flags)
{
	struct msghdr msg;
	int32_t ret = 0;
	int32_t errsv;

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = iovlen;

	ret = sendmsg(handle->opfd, &msg, flags);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: sendmsg syscall returned %d (errno: %d)",
		    ret, errsv);

	return (ret >= 0) ? ret : -errsv;
}

static inline int32_t _kcapi_common_vmsplice_iov(struct kcapi_handle *handle,
						 struct iovec *iov,
						 unsigned long iovlen,
						 uint32_t flags)
{
	int32_t ret = 0;
	uint32_t inlen = 0;
	unsigned long i;
	int32_t errsv;

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	for (i = 0; i < iovlen; i++)
		inlen += iov[i].iov_len;

	/* kernel processes input data with max size of one page */
	handle->processed_sg += ((inlen + sysconf(_SC_PAGESIZE) - 1) /
				 sysconf(_SC_PAGESIZE));
	if (handle->processed_sg > ALG_MAX_PAGES)
		return _kcapi_common_send_data(handle, iov, iovlen,
					       (flags & SPLICE_F_MORE) ?
					        MSG_MORE : 0);

	ret = vmsplice(handle->pipes[1], iov, iovlen, SPLICE_F_GIFT|flags);
	if (0 > ret)
		return ret;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: vmsplice syscall returned %d (errno: %d)",
		    ret, errno);

	if ((uint32_t)ret != inlen) {
		kcapi_dolog(LOG_ERR, "vmsplice: not all data received by kernel (data recieved: %ld -- data sent: %lu)",
			(long)ret, (unsigned long)inlen);
		return -EFAULT;
	}
	ret = splice(handle->pipes[0], NULL, handle->opfd, NULL, ret, flags);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: splice syscall returned %d (errno: %d)",
		    ret, errsv);

	return (ret >= 0) ? ret : -errsv;
}

static inline int32_t _kcapi_common_vmsplice_chunk(struct kcapi_handle *handle,
						   const uint8_t *in,
						   uint32_t inlen,
						   uint32_t flags)
{
	struct iovec iov;
	uint32_t processed = 0;
	int ret = 0;

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	while (inlen) {
		int32_t ret = 0;

		iov.iov_base = (void*)(uintptr_t)(in + processed);
		iov.iov_len = inlen;

		if ((handle->processed_sg++) > ALG_MAX_PAGES) {
			ret = _kcapi_common_send_data(handle, &iov, 1,
						      (flags & SPLICE_F_MORE) ?
						       MSG_MORE : 0);
		} else {
			ret = vmsplice(handle->pipes[1], &iov, 1,
				       SPLICE_F_GIFT|flags);
			kcapi_dolog(LOG_DEBUG, "AF_ALG: vmsplice syscall returned %d (errno: %d)",
				    ret, errno);
			if (0 > ret)
				return ret;
			ret = splice(handle->pipes[0], NULL, handle->opfd,
				     NULL, ret, flags);
			kcapi_dolog(LOG_DEBUG, "AF_ALG: splice syscall returned %d (errno: %d)",
				    ret, errno);
		}
		if (0 > ret)
			return ret;

		processed += ret;
		inlen -= ret;
	}

	return processed;
}

static inline int32_t _kcapi_common_recv_data(struct kcapi_handle *handle,
					      struct iovec *iov,
					      uint32_t iovlen)
{
	struct msghdr msg;
	int32_t ret = 0;
	int32_t errsv = 0;

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = iovlen;

	ret = recvmsg(handle->opfd, &msg, 0);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: recvmsg syscall returned %d (errno: %d)",
		    ret, errsv);

#if 0
	/*
	 * Truncated message digests can be identified with this check.
	 */
	if (msg.msg_flags & MSG_TRUNC) {
		fprintf(stderr, "recvmsg: processed data was truncated by kernel (only %lu bytes processed)\n", (unsigned long)ret);
		return -EMSGSIZE;
	}
#endif

	/*
	 * As the iovecs are processed and removed from the list in the kernel
	 * we can also reset the list of processed iovecs here.
	 *
	 * Note, if there is an error, the kernel keeps the list unless it is
	 * a "valid" error of EBADMSG indicating an integrity error of the
	 * crypto operation.
	 */
	if (ret >= 0 || errsv == EBADMSG)
		handle->processed_sg = 0;

	return (ret >= 0) ? ret : -errsv;
}

static inline int32_t _kcapi_common_read_data(struct kcapi_handle *handle,
					      uint8_t *out, uint32_t outlen)
{
	int32_t ret = 0;
	int32_t errsv;

	ret = _kcapi_common_accept(handle);
	if (ret)
		return ret;

	ret = read(handle->opfd, out, outlen);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: read syscall returned %d (errno: %d)",
		    ret, errsv);

	return (ret >= 0) ? ret : -errsv;
}

static inline int _kcapi_common_setkey(struct kcapi_handle *handle,
				       const uint8_t *key, uint32_t keylen)
{
	int ret = 0;
	int errsv;

	ret = setsockopt(handle->tfmfd, SOL_ALG, ALG_SET_KEY, key, keylen);
	errsv = errno;
	kcapi_dolog(LOG_DEBUG, "AF_ALG: sendmsg syscall returned %d (errno: %d)",
		    ret, errsv);

	return (ret >= 0) ? ret : -errsv;
}

static int __kcapi_common_getinfo(struct kcapi_handle *handle,
				  const char *ciphername,
				  int drivername)
{
	int ret = -EFAULT;
	int errsv = 0;

	/* NETLINK_CRYPTO specific */
	char buf[4096];
	struct nlmsghdr *res_n = (struct nlmsghdr *)buf;
	struct {
		struct nlmsghdr n;
		struct crypto_user_alg cru;
	} req;
	struct crypto_user_alg *cru_res;
	int res_len = 0;
	struct rtattr *tb[CRYPTOCFGA_MAX+1];
	struct rtattr *rta;

	/* AF_NETLINK specific */
	struct sockaddr_nl nl;
	int sd = 0;
	socklen_t addr_len;
	struct iovec iov;
	struct msghdr msg;

	memset(&req, 0, sizeof(req));
	memset(&buf, 0, sizeof(buf));
	memset(&msg, 0, sizeof(msg));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.cru));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = CRYPTO_MSG_GETALG;
	req.n.nlmsg_seq = time(NULL);

	if (drivername)
		strncpy(req.cru.cru_driver_name, ciphername,
			strlen(ciphername));
	else
		strncpy(req.cru.cru_name, ciphername, strlen(ciphername));

	/* talk to netlink socket */
	sd =  socket(AF_NETLINK, SOCK_RAW, NETLINK_CRYPTO);
	if (sd < 0) {
		kcapi_dolog(LOG_ERR, "Netlink error: cannot open netlink socket");
		return -errno;
	}
	memset(&nl, 0, sizeof(nl));
	nl.nl_family = AF_NETLINK;
	if (bind(sd, (struct sockaddr*)&nl, sizeof(nl)) < 0) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "Netlink error: cannot bind netlink socket");
		goto out;
	}
	/* sanity check that netlink socket was successfully opened */
	addr_len = sizeof(nl);
	if (getsockname(sd, (struct sockaddr*)&nl, &addr_len) < 0) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "Netlink error: cannot getsockname");
		goto out;
	}
	if (addr_len != sizeof(nl)) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "Netlink error: wrong address length %d",
			addr_len);
		goto out;
	}
	if (nl.nl_family != AF_NETLINK) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "Netlink error: wrong address family %d",
			nl.nl_family);
		goto out;
	}

	/* sending data */
	memset(&nl, 0, sizeof(nl));
	nl.nl_family = AF_NETLINK;
	iov.iov_base = (void*) &req.n;
	iov.iov_len = req.n.nlmsg_len;
	msg.msg_name = &nl;
	msg.msg_namelen = sizeof(nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	if (sendmsg(sd, &msg, 0) < 0) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "Netlink error: sendmsg failed");
		goto out;
	}
	memset(buf,0,sizeof(buf));
	iov.iov_base = buf;
	while (1) {
		iov.iov_len = sizeof(buf);
		ret = recvmsg(sd, &msg, 0);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			errsv = errno;
			kcapi_dolog(LOG_ERR, "Netlink error: netlink receive error");
			goto out;
		}
		if (ret == 0) {
			errsv = errno;
			kcapi_dolog(LOG_ERR, "Netlink error: no data");
			goto out;
		}
		if ((uint32_t)ret > sizeof(buf)) {
			errsv = errno;
			kcapi_dolog(LOG_ERR, "Netlink error: received too much data");
			goto out;
		}
		break;
	}

	ret = -EFAULT;
	res_len = res_n->nlmsg_len;
	if (res_n->nlmsg_type == NLMSG_ERROR) {
		/*
		 * return -EAGAIN -- this error will occur if we received a
		 * driver name, but used it for a generic name. Allow caller
		 * to invoke function again where driver name is looked up
		 */
		ret = -EAGAIN;
		goto out;
	}

	if (res_n->nlmsg_type == CRYPTO_MSG_GETALG) {
		cru_res = NLMSG_DATA(res_n);
		res_len -= NLMSG_SPACE(sizeof(*cru_res));
	}
	if (res_len < 0) {
		kcapi_dolog(LOG_ERR, "Netlink error: nlmsg len %d", res_len);
		goto out;
	}

	/* parse data */
	rta = CR_RTA(cru_res);
	memset(tb, 0, sizeof(struct rtattr *) * (CRYPTOCFGA_MAX + 1));
	while (RTA_OK(rta, res_len)) {
		if ((rta->rta_type <= CRYPTOCFGA_MAX) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, res_len);
	}
	if (res_len) {
		kcapi_dolog(LOG_ERR, "Netlink error: unprocessed data %d",
			    res_len);
		goto out;
	}

	if (tb[CRYPTOCFGA_REPORT_HASH]) {
		struct rtattr *rta = tb[CRYPTOCFGA_REPORT_HASH];
		struct crypto_report_hash *rsh =
			(struct crypto_report_hash *) RTA_DATA(rta);
		handle->info.hash_digestsize = rsh->digestsize;
		handle->info.blocksize = rsh->blocksize;
		kcapi_dolog(LOG_DEBUG, "Get cipher info: hash with digestsize %u,  blocksize %u",
			    rsh->digestsize, rsh->blocksize);
	}
	if (tb[CRYPTOCFGA_REPORT_BLKCIPHER]) {
		struct rtattr *rta = tb[CRYPTOCFGA_REPORT_BLKCIPHER];
		struct crypto_report_blkcipher *rblk =
			(struct crypto_report_blkcipher *) RTA_DATA(rta);
		handle->info.blocksize = rblk->blocksize;
		handle->info.ivsize = rblk->ivsize;
		handle->info.blk_min_keysize = rblk->min_keysize;
		handle->info.blk_max_keysize = rblk->max_keysize;
		kcapi_dolog(LOG_DEBUG, "Get cipher info: block cipher with blocksize %u, ivsize %u, minimum keysize %u, maximum keysize %u",
			    rblk->blocksize, rblk->ivsize, rblk->min_keysize,
			    rblk->max_keysize);
	}
	if (tb[CRYPTOCFGA_REPORT_AEAD]) {
		struct rtattr *rta = tb[CRYPTOCFGA_REPORT_AEAD];
		struct crypto_report_aead *raead =
			(struct crypto_report_aead *) RTA_DATA(rta);
		handle->info.blocksize = raead->blocksize;
		handle->info.ivsize = raead->ivsize;
		handle->info.aead_maxauthsize = raead->maxauthsize;
		kcapi_dolog(LOG_DEBUG, "Get cipher info: AEAD block cipher with blocksize %u, ivsize %u, maximum authentication size %u",
			    raead->blocksize, raead->ivsize,
			    raead->maxauthsize);
	}
	if (tb[CRYPTOCFGA_REPORT_RNG]) {
		struct rtattr *rta = tb[CRYPTOCFGA_REPORT_RNG];
		struct crypto_report_rng *rrng =
			(struct crypto_report_rng *) RTA_DATA(rta);
		handle->info.rng_seedsize = rrng->seedsize;
		kcapi_dolog(LOG_DEBUG, "Get cipher info: RNG cipher with seedsize %u",
			    rrng->seedsize);
	}
	if (tb[CRYPTOCFGA_UNSPEC])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: unspecified data received");
	if (tb[CRYPTOCFGA_PRIORITY_VAL])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: u32 value received");
	if (tb[CRYPTOCFGA_REPORT_LARVAL])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: larval value received");
	if (tb[CRYPTOCFGA_REPORT_COMPRESS])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: compression algorithm type received");
	if (tb[CRYPTOCFGA_REPORT_CIPHER])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: simple cipher algorithm type received");
	if (tb[CRYPTOCFGA_REPORT_AKCIPHER])
		kcapi_dolog(LOG_DEBUG, "Get cipher info: asymmetric cipher algorithm type received");
	kcapi_dolog(LOG_VERBOSE, "Get cipher info: all information for %s received from kernel",
		    ciphername);

	ret = 0;

out:
	close(sd);
	return (errsv) ? -errsv : ret;
}

static int _kcapi_common_getinfo(struct kcapi_handle *handle,
				 const char *ciphername)
{
	int ret = __kcapi_common_getinfo(handle, ciphername, 0);
	if (ret)
		return __kcapi_common_getinfo(handle, ciphername, 1);
	return 0;
}

/* Initialize AIO -- on error, AIO is simply not used, but lib lives on */
static void _kcapi_aio_init(struct kcapi_handle *handle)
{
	if (handle->aio.skcipher_aio_disable)
		return;

	handle->aio.efd = eventfd(0, EFD_CLOEXEC);
	if (handle->aio.efd < 0)
		goto err;

	if (io_setup(KCAPI_AIO_CONCURRENT, &handle->aio.aio_ctx) < 0) {
		kcapi_dolog(LOG_ERR, "io_setup error");
		goto err;
	}

	kcapi_dolog(LOG_VERBOSE, "asynchronoous I/O initialized");

	return;

err:
	handle->aio.skcipher_aio_disable = 1;
	if (handle->aio.efd != -1)
		close(handle->aio.efd);
	handle->aio.efd = -1;
	return;
}

static inline void _kcapi_aio_destroy(struct kcapi_handle *handle)
{
	if (handle->aio.skcipher_aio_disable)
		return;
	if (handle->aio.efd != -1)
		close(handle->aio.efd);
	io_destroy(handle->aio.aio_ctx);
}

static inline void _kcapi_handle_destroy(struct kcapi_handle *handle)
{
	if (handle->tfmfd != -1)
		close(handle->tfmfd);
	if (handle->opfd != -1)
		close(handle->opfd);
	if (handle->pipes[0] != -1)
		close(handle->pipes[0]);
	if (handle->pipes[1] != -1)
		close(handle->pipes[1]);
	_kcapi_aio_destroy(handle);
	memset(handle, 0, sizeof(struct kcapi_handle));
}

static int _kcapi_handle_init(struct kcapi_handle **caller, const char *type,
			      const char *ciphername, uint32_t flags)
{
	struct sockaddr_alg sa;
	struct kcapi_handle *handle;
	int ret;
	int errsv = 0;
	char versionbuffer[50];

	kcapi_versionstring(versionbuffer, sizeof(versionbuffer));
	kcapi_dolog(LOG_VERBOSE, "%s - initializing cipher operation with kernel",
		    versionbuffer);

	handle = calloc(1, sizeof(struct kcapi_handle));
	if (!handle)
		return -ENOMEM;

	memset(&sa, 0, sizeof(sa));
	sa.salg_family = AF_ALG;
	snprintf((char *)sa.salg_type, sizeof(sa.salg_type),"%s", type);
	snprintf((char *)sa.salg_name, sizeof(sa.salg_name),"%s", ciphername);

	handle->tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	kcapi_dolog(LOG_DEBUG, "AF_ALG: socket syscall %s (errno: %d)",
		    (handle->tfmfd == -1) ? "failed": "passed", errno);
	if (handle->tfmfd == -1) {
		errsv = -EOPNOTSUPP;
		goto err;
	}

	if (bind(handle->tfmfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "AF_ALG: bind failed (errno: %d)",
			    errsv);
		close(handle->tfmfd);
		handle->tfmfd = -1;
		goto err;
	}
	kcapi_dolog(LOG_DEBUG, "AF_ALG: bind syscall passed (errno: %d)",
		    errno);

	handle->opfd = -1;

	ret = pipe(handle->pipes);
	if (ret) {
		errsv = errno;
		kcapi_dolog(LOG_DEBUG, "AF_ALG: pipe syscall failed (errno: %d)",
			    errsv);
		close(handle->tfmfd);
		goto err;
	}
	kcapi_dolog(LOG_DEBUG, "AF_ALG: pipe syscall passed");

	ret = _kcapi_common_getinfo(handle, ciphername);
	if (ret) {
		errsv = errno;
		kcapi_dolog(LOG_ERR, "NETLINK_CRYPTO: cannot obtain cipher information for %s (is required crypto_user.c patch missing? see documentation)",
			    ciphername);
		goto err;
	}

	if (flags & KCAPI_INIT_AIO)
		_kcapi_aio_init(handle);
	else
		handle->aio.skcipher_aio_disable = 1;

	kcapi_dolog(LOG_VERBOSE, "communication for %s with kernel initialized", ciphername);

	*caller = handle;

	return ret;

err:
	_kcapi_handle_destroy(handle);
	return -errsv;
}

/*********** Generic Helper functions *************************/

void kcapi_memset_secure(void *s, int c, uint32_t n)
{
	memset(s, c, n);
	__asm__ __volatile__("" : : "r" (s) : "memory");
}

void kcapi_versionstring(char *buf, uint32_t buflen)
{
	snprintf(buf, buflen, "libkcapi %d.%d.%d", MAJVERSION, MINVERSION,
		 PATCHLEVEL);
}

uint32_t kcapi_version(void)
{
	uint32_t version = 0;

	version =  MAJVERSION * 1000000;
	version += MINVERSION * 10000;
	version += PATCHLEVEL * 100;

	return version;
}

int kcapi_pad_iv(struct kcapi_handle *handle,
		 const uint8_t *iv, uint32_t ivlen,
		 uint8_t **newiv, uint32_t *newivlen)
{
	uint8_t *niv = NULL;
	uint32_t nivlen = handle->info.ivsize;
	uint32_t copylen = (ivlen > nivlen) ? nivlen : ivlen;
	int ret = 0;

	ret = posix_memalign((void *)&niv, 16, nivlen);
	if (ret)
		return -ret;
	memcpy(niv, iv, copylen);
	if (nivlen > copylen)
		memset(niv + copylen, 0, nivlen - copylen);

	*newiv = niv;
	*newivlen = nivlen;

	return 0;
}

int kcapi_cipher_init(struct kcapi_handle **handle, const char *ciphername,
		      uint32_t flags)
{
	return _kcapi_handle_init(handle, "skcipher", ciphername, flags);
}

void kcapi_cipher_destroy(struct kcapi_handle *handle)
{
	if (!handle)
		return;
	_kcapi_handle_destroy(handle);
	kcapi_memset_secure(handle, 0, sizeof(struct kcapi_handle));
}

int kcapi_cipher_setkey(struct kcapi_handle *handle,
			const uint8_t *key, uint32_t keylen)
{
	return _kcapi_common_setkey(handle, key, keylen);
}

static inline int32_t _kcapi_aio_read(struct kcapi_handle *handle,
				     uint8_t *out, uint32_t outlen)
{
	(void)handle;
	(void)out;
	(void)outlen;
	kcapi_dolog(LOG_ERR, "AIO support not complete");
	return -EOPNOTSUPP;
#if 0
	struct iocb *cb = NULL;
	int r = 0;

	cb = &cbt[handle->aio.curr_cio];
        while (cb->aio_fildes)
		poll_data(10);

        memset(cb, 0, sizeof(*cb));
        cb->aio_fildes = handle->opfd;
        cb->aio_lio_opcode = IOCB_CMD_PREAD;
        cb->aio_buf = out;
        cb->aio_offset = 0;
        cb->aio_data = i;
        cb->aio_nbytes = outlen;
        cb->aio_flags = IOCB_FLAG_RESFD;
        cb->aio_resfd = handle->aio.efd;
        r = io_read(handle->aio.aio_ctx, 1, &cb);
        if (r != 1) {
		if (r < 0) {
			fprintf(stderr, "io_read Error: %d\n", errno);
			return -EFAULT;
		} else {
			fprintf(stderr, "Could not sumbit AIO read\n");
			return -EIO;
		}
	}

        if (handle->aio.curr_cio) >= KCAPI_AIO_CONCURRENT / 2)
		poll_data(1);

	return 0;
#endif
}

static int32_t _kcapi_cipher_crypt(struct kcapi_handle *handle,
				   const uint8_t *in, uint32_t inlen,
				   uint8_t *out, uint32_t outlen,
				   int access, int enc)
{
	struct iovec iov;
	int32_t ret = 0;

	if (!in || !inlen || !out || !outlen) {
		kcapi_dolog(LOG_ERR,
			    "Symmetric Encryption: Empty plaintext or ciphertext buffer provided");
		return -EINVAL;
	}

	/*
	 * Using two syscalls with memcpy is faster than four syscalls
	 * without memcpy below the given threshold.
	 */
	if ((access == KCAPI_ACCESS_HEURISTIC && inlen <= (1<<13)) ||
	    access == KCAPI_ACCESS_SENDMSG) {
		iov.iov_base = (void*)(uintptr_t)in;
		iov.iov_len = inlen;
		ret = _kcapi_common_send_meta(handle, &iov, 1, enc, 0);
		if (0 > ret)
			return ret;
	} else {
		ret = _kcapi_common_send_meta(handle, NULL, 0, enc, 0);
		if (0 > ret)
			return ret;
		ret = _kcapi_common_vmsplice_chunk(handle, in, inlen, 0);
		if (0 > ret)
			return ret;
	}

	if (handle->aio.skcipher_aio_disable) {
		return _kcapi_common_read_data(handle, out, outlen);
	} else {
		ret = _kcapi_aio_read(handle, out, outlen);
		if (ret == -EIO) {
			/* AIO not available for specific interface */
			handle->aio.skcipher_aio_disable = 1;
			_kcapi_aio_destroy(handle);
			return _kcapi_common_read_data(handle, out, outlen);
		}
		return ret;
	}
}

static int32_t _kcapi_cipher_crypt_chunk(struct kcapi_handle *handle,
					 const uint8_t *in, uint32_t inlen,
					 uint8_t *out, uint32_t outlen,
					int access, int enc)
{
	int32_t totallen = 0;

	while (inlen) {
		uint32_t process = inlen;
		int32_t ret = 0;

		/*
		 * We do not check that sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES is
		 * a multiple of blocksize, because we assume that this is
		 * always the case.
		 */
		if (inlen > (uint32_t)(sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES))
			process = sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES;

		ret = _kcapi_cipher_crypt(handle, in, process, out, outlen,
					  access, enc);
		if (ret < 0)
			return ret;

		totallen += process;
		in += process;
		inlen -= process;
		out += ret;
		outlen -= ret;
	}

	return totallen;
}

int32_t kcapi_cipher_encrypt(struct kcapi_handle *handle,
			     const uint8_t *in, uint32_t inlen,
			     const uint8_t *iv,
			     uint8_t *out, uint32_t outlen, int access)
{
	uint32_t bs = handle->info.blocksize;

	/* require properly sized output data size */
	if (outlen < ((inlen + bs - 1) / bs * bs)) {
		kcapi_dolog(LOG_ERR,
			    "Symmetric Encryption: Ciphertext buffer (%lu) is not plaintext buffer (%lu) rounded up to multiple of block size %u",
			    (unsigned long) outlen, (unsigned long)inlen, bs);
		return -EINVAL;
	}

	handle->cipher.iv = iv;
	return _kcapi_cipher_crypt_chunk(handle, in, inlen, out, outlen, access,
					 ALG_OP_ENCRYPT);
}

int32_t kcapi_cipher_decrypt(struct kcapi_handle *handle,
			     const uint8_t *in, uint32_t inlen,
			     const uint8_t *iv,
			     uint8_t *out, uint32_t outlen, int access)
{
	/* require properly sized output data size */
	if (inlen % handle->info.blocksize) {
		kcapi_dolog(LOG_ERR,
			    "Symmetric Decryption: Ciphertext buffer is not multiple of block size %u",
			    handle->info.blocksize);
		return -EINVAL;
	}

	if (outlen < inlen) {
		kcapi_dolog(LOG_ERR,
			    "Symmetric Decryption: Plaintext buffer (%lu) is smaller as ciphertext buffer (%lu)",
			    (unsigned long)outlen, (unsigned long)inlen);
		return -EINVAL;
	}

	handle->cipher.iv = iv;
	return _kcapi_cipher_crypt_chunk(handle, in, inlen, out, outlen, access,
					 ALG_OP_DECRYPT);
}

int32_t kcapi_cipher_stream_init_enc(struct kcapi_handle *handle,
				     const uint8_t *iv,
				     struct iovec *iov, uint32_t iovlen)
{
	handle->cipher.iv = iv;
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_ENCRYPT,
				       MSG_MORE);
}

int32_t kcapi_cipher_stream_init_dec(struct kcapi_handle *handle,
				     const uint8_t *iv,
				     struct iovec *iov, uint32_t iovlen)
{
	handle->cipher.iv = iv;
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_DECRYPT,
				       MSG_MORE);
}

int32_t kcapi_cipher_stream_update(struct kcapi_handle *handle,
				   struct iovec *iov, uint32_t iovlen)
{
	if (handle->processed_sg <= ALG_MAX_PAGES)
		return _kcapi_common_vmsplice_iov(handle, iov, iovlen,
						  SPLICE_F_MORE);
	else
		return _kcapi_common_send_data(handle, iov, iovlen, MSG_MORE);
}

int32_t kcapi_cipher_stream_op(struct kcapi_handle *handle,
			       struct iovec *iov, uint32_t iovlen)
{
	if (!iov || !iovlen) {
		kcapi_dolog(LOG_ERR,
			    "Symmetric operation: No buffer for output data provided");
		return -EINVAL;
	}
	return _kcapi_common_recv_data(handle, iov, iovlen);
}

uint32_t kcapi_cipher_ivsize(struct kcapi_handle *handle)
{
	return handle->info.ivsize;
}

uint32_t kcapi_cipher_blocksize(struct kcapi_handle *handle)
{
	return handle->info.blocksize;
}

int kcapi_aead_init(struct kcapi_handle **handle, const char *ciphername,
		    uint32_t flags)
{
	return _kcapi_handle_init(handle, "aead", ciphername, flags);
}

void kcapi_aead_destroy(struct kcapi_handle *handle)
{
	_kcapi_handle_destroy(handle);
}

int kcapi_aead_setkey(struct kcapi_handle *handle,
		      const uint8_t *key, uint32_t keylen)
{
	return _kcapi_common_setkey(handle, key, keylen);
}

int kcapi_aead_settaglen(struct kcapi_handle *handle, uint32_t taglen)
{
	handle->aead.tag = NULL;
	handle->aead.taglen = taglen;
	if (setsockopt(handle->tfmfd, SOL_ALG, ALG_SET_AEAD_AUTHSIZE,
		       NULL, taglen) == -1)
		return -EINVAL;

	return 0;
}

void kcapi_aead_setassoclen(struct kcapi_handle *handle, uint32_t assoclen)
{
	handle->aead.assoclen = assoclen;
}

void kcapi_aead_getdata(struct kcapi_handle *handle,
			uint8_t *encdata, uint32_t encdatalen,
			uint8_t **aad, uint32_t *aadlen,
			uint8_t **data, uint32_t *datalen,
			uint8_t **tag, uint32_t *taglen)
{
	if (encdatalen <  handle->aead.taglen + handle->aead.assoclen) {
		kcapi_dolog(LOG_ERR, "Result of encryption operation (%lu) is smaller than tag and AAD length (%lu)",
			    (unsigned long)encdatalen,
			    (unsigned long)handle->aead.taglen +
			    (unsigned long)handle->aead.assoclen);
		return;
	}
	if (aad) {
		*aad = encdata;
		*aadlen = handle->aead.assoclen;
	}
	if (data) {
		*data = encdata + handle->aead.assoclen;
		*datalen = encdatalen - handle->aead.assoclen -
			   handle->aead.taglen;
	}
	if (tag) {
		*tag = encdata + encdatalen - handle->aead.taglen;
		*taglen = handle->aead.taglen;
	}
}

int32_t kcapi_aead_encrypt(struct kcapi_handle *handle,
			   uint8_t *in, uint32_t inlen,
			   const uint8_t *iv,
			   uint8_t *out, uint32_t outlen, int access)
{
	struct iovec iov;
	int32_t ret = 0;

	/* require properly sized output data size */
	if (outlen < inlen) {
		kcapi_dolog(LOG_ERR,
			    "AEAD Encryption: Ciphertext buffer (%u) is smaller than plaintext buffer (%u)",
			    outlen, inlen);
		return -EINVAL;
	}

	if (inlen > (uint32_t)(sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES)) {
		kcapi_dolog(LOG_ERR,
			    "AEAD Encryption: Plaintext buffer (%u) is larger than maximum chunk size (%lu)",
			    inlen, sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES);
		return -EMSGSIZE;
	}

	handle->cipher.iv = iv;

	iov.iov_base = in;
	iov.iov_len = inlen;
	if (access == KCAPI_ACCESS_HEURISTIC ||
	    access == KCAPI_ACCESS_SENDMSG) {
		ret = _kcapi_common_send_meta(handle, &iov, 1,
					      ALG_OP_ENCRYPT, 0);
		if (0 > ret)
			return ret;
	} else {
		ret = _kcapi_common_send_meta(handle, NULL, 0, ALG_OP_ENCRYPT,
			inlen ? MSG_MORE : 0);
		if (0 > ret)
			return ret;
		ret = _kcapi_common_vmsplice_iov(handle, &iov, 1, 0);
		if (0 > ret)
			return ret;
	}

	ret = _kcapi_common_read_data(handle, out, outlen);
	if (ret < 0)
		return ret;
	if ((ret < (int32_t)handle->aead.taglen))
		return -E2BIG;

	return ret;
}

int32_t kcapi_aead_decrypt(struct kcapi_handle *handle,
			   uint8_t *in, uint32_t inlen,
			   const uint8_t *iv,
			   uint8_t *out, uint32_t outlen, int access)
{
	struct iovec iov;
	int32_t ret = 0;

	/* require properly sized output data size */
	if (outlen < inlen) {
		kcapi_dolog(LOG_ERR,
			    "AEAD Decryption: Plaintext buffer (%u) is smaller than ciphertext buffer (%u)",
			    outlen, inlen);
		return -EINVAL;
	}

	if (inlen > (uint32_t)(sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES)) {
		kcapi_dolog(LOG_ERR,
			    "AEAD Decryption: Ciphertext buffer (%u) is larger than maximum chunk size (%lu)",
			    inlen, sysconf(_SC_PAGESIZE) * ALG_MAX_PAGES);
		return -EMSGSIZE;
	}

	handle->cipher.iv = iv;

	iov.iov_base = in;
	iov.iov_len = inlen;
	if (access == KCAPI_ACCESS_HEURISTIC ||
	    access == KCAPI_ACCESS_SENDMSG) {
		ret = _kcapi_common_send_meta(handle, &iov, 1,
					      ALG_OP_DECRYPT, 0);
		if (0 > ret)
			return ret;
	} else {
		ret = _kcapi_common_send_meta(handle, NULL, 0, ALG_OP_DECRYPT,
					      MSG_MORE);
		if (0 > ret)
			return ret;
		ret = _kcapi_common_vmsplice_iov(handle, &iov, 1, 0);
		if (0 > ret)
			return ret;
	}

	/* handle null test vectors */
	if (outlen) {
		return _kcapi_common_read_data(handle, out, outlen);
	} else {
		iov.iov_base = out;
		iov.iov_len = outlen;
		return _kcapi_common_recv_data(handle, &iov, 1);
	}
}

int32_t kcapi_aead_stream_init_enc(struct kcapi_handle *handle,
				   const uint8_t *iv,
				   struct iovec *iov, uint32_t iovlen)
{
	handle->cipher.iv = iv;
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_ENCRYPT,
				       MSG_MORE);
}

int32_t kcapi_aead_stream_init_dec(struct kcapi_handle *handle,
				   const uint8_t *iv,
				   struct iovec *iov, uint32_t iovlen)
{
	handle->cipher.iv = iv;
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_DECRYPT,
				       MSG_MORE);
}

int32_t kcapi_aead_stream_update(struct kcapi_handle *handle,
				 struct iovec *iov, uint32_t iovlen)
{
	if (handle->processed_sg <= ALG_MAX_PAGES)
		return _kcapi_common_vmsplice_iov(handle, iov, iovlen,
						  SPLICE_F_MORE);
	else
		return _kcapi_common_send_data(handle, iov, iovlen, MSG_MORE);
}

int32_t kcapi_aead_stream_update_last(struct kcapi_handle *handle,
				      struct iovec *iov, uint32_t iovlen)
{
	if (handle->processed_sg <= ALG_MAX_PAGES)
		return _kcapi_common_vmsplice_iov(handle, iov, iovlen, 0);
	else
		return _kcapi_common_send_data(handle, iov, iovlen, 0);
}

int32_t kcapi_aead_stream_op(struct kcapi_handle *handle,
			     struct iovec *iov, uint32_t iovlen)
{
	if (!iov) {
		kcapi_dolog(LOG_ERR,
			    "AEAD operation: No buffer for output data provided");
		return -EINVAL;
	}

	return _kcapi_common_recv_data(handle, iov, iovlen);
}

uint32_t kcapi_aead_ivsize(struct kcapi_handle *handle)
{
	return handle->info.ivsize;
}

uint32_t kcapi_aead_blocksize(struct kcapi_handle *handle)
{
	return handle->info.blocksize;
}

uint32_t kcapi_aead_authsize(struct kcapi_handle *handle)
{
	return handle->info.aead_maxauthsize;
}

uint32_t kcapi_aead_outbuflen(struct kcapi_handle *handle,
			    uint32_t inlen, uint32_t assoclen, uint32_t taglen)
{
	int bs = handle->info.blocksize;

	return ((inlen + bs - 1) / bs * bs + taglen + assoclen);
}

int kcapi_aead_ccm_nonce_to_iv(const uint8_t *nonce, uint32_t noncelen,
			       uint8_t **iv, uint32_t *ivlen)
{
	uint8_t *newiv = NULL;
	uint8_t l = 16 - 2 - noncelen;
	int ret = 0;

	if (noncelen > 16 - 2)
		return -EINVAL;
	ret = posix_memalign((void *)&newiv, 16, 16);
	if (ret)
		return -ret;
	memset(newiv, 0, 16);
	newiv[0] = l;
	memcpy(newiv + 1, nonce, noncelen);

	*iv = newiv;
	*ivlen = 16;

	return 0;
}

int kcapi_md_init(struct kcapi_handle **handle, const char *ciphername,
		  uint32_t flags)
{
	return _kcapi_handle_init(handle, "hash", ciphername, flags);
}

void kcapi_md_destroy(struct kcapi_handle *handle)
{
	_kcapi_handle_destroy(handle);
}

int kcapi_md_setkey(struct kcapi_handle *handle,
		    const uint8_t *key, uint32_t keylen)
{
	return _kcapi_common_setkey(handle, key, keylen);
}

static inline int32_t _kcapi_md_update(struct kcapi_handle *handle,
				       const uint8_t *buffer, uint32_t len)
{
	int32_t ret = 0;

	/* zero buffer length cannot be handled via splice */
	if (len < (1<<15)) {
		ret = _kcapi_common_accept(handle);
		if (ret)
			return ret;
		ret = send(handle->opfd, buffer, len, MSG_MORE);
	} else {
		ret = _kcapi_common_vmsplice_chunk(handle, buffer, len,
						   SPLICE_F_MORE);
	}

	if (ret < 0 || (uint32_t)ret < len)
		return -EIO;
	return 0;
}

int32_t kcapi_md_update(struct kcapi_handle *handle,
			const uint8_t *buffer, uint32_t len)
{
	return _kcapi_md_update(handle, buffer, len);
}

static int32_t _kcapi_md_final(struct kcapi_handle *handle,
			       uint8_t *buffer, uint32_t len)
{
	struct iovec iov;

	if (!buffer || !len) {
		kcapi_dolog(LOG_ERR,
			    "Message digest: output buffer too small (seen %lu - required %u)",
			    (unsigned long)len,	handle->info.hash_digestsize);
		return -EINVAL;
	}

	iov.iov_base = (void*)(uintptr_t)buffer;
	iov.iov_len = len;
	return _kcapi_common_recv_data(handle, &iov, 1);
}

int32_t kcapi_md_final(struct kcapi_handle *handle,
		       uint8_t *buffer, uint32_t len)
{
	return _kcapi_md_final(handle, buffer, len);
}

int32_t kcapi_md_digest(struct kcapi_handle *handle,
		       const uint8_t *in, uint32_t inlen,
		       uint8_t *out, uint32_t outlen)
{
	int32_t ret = 0;

	ret = _kcapi_md_update(handle, in, inlen);
	if (0 > ret)
		return ret;
	return _kcapi_md_final(handle, out, outlen);
}

uint32_t kcapi_md_digestsize(struct kcapi_handle *handle)
{
	return handle->info.hash_digestsize;
}

uint32_t kcapi_md_blocksize(struct kcapi_handle *handle)
{
	return handle->info.blocksize;
}

int kcapi_rng_init(struct kcapi_handle **handle, const char *ciphername,
		   uint32_t flags)
{
	return _kcapi_handle_init(handle, "rng", ciphername, flags);
}

void kcapi_rng_destroy(struct kcapi_handle *handle)
{
	_kcapi_handle_destroy(handle);
}

int kcapi_rng_seed(struct kcapi_handle *handle, uint8_t *seed,
		   uint32_t seedlen)
{
	return _kcapi_common_setkey(handle, seed, seedlen);
}

int32_t kcapi_rng_generate(struct kcapi_handle *handle,
			   uint8_t *buffer, uint32_t len)
{
	int32_t out = 0;
	struct iovec iov;

	while (len) {
		int32_t r = 0;

		iov.iov_base = (void*)(uintptr_t)buffer;
		iov.iov_len = len;
		r = _kcapi_common_recv_data(handle, &iov, 1);
		if (0 >= r)
			return r;
		len -= r;
		out += r;

		buffer += r;
	}

	return out;
}

int kcapi_akcipher_init(struct kcapi_handle **handle, const char *ciphername,
			uint32_t flags)
{
	return _kcapi_handle_init(handle, "akcipher", ciphername, flags);
}

void kcapi_akcipher_destroy(struct kcapi_handle *handle)
{
	_kcapi_handle_destroy(handle);
}

int kcapi_akcipher_setkey(struct kcapi_handle *handle,
			  const uint8_t *key, uint32_t keylen)
{
	return _kcapi_common_setkey(handle, key, keylen);
}

int kcapi_akcipher_setpubkey(struct kcapi_handle *handle,
			     const uint8_t *key, uint32_t keylen)
{
	int ret = 0;

	ret = setsockopt(handle->tfmfd, SOL_ALG, ALG_SET_PUBKEY, key, keylen);
	return (ret >= 0) ? ret : -errno;
}

int32_t kcapi_akcipher_encrypt(struct kcapi_handle *handle,
			       const uint8_t *in, uint32_t inlen,
			       uint8_t *out, uint32_t outlen, int access)
{
	return _kcapi_cipher_crypt(handle, in, inlen, out, outlen, access,
				   ALG_OP_ENCRYPT);
}

int32_t kcapi_akcipher_decrypt(struct kcapi_handle *handle,
			       const uint8_t *in, uint32_t inlen,
			       uint8_t *out, uint32_t outlen, int access)
{
	return _kcapi_cipher_crypt(handle, in, inlen, out, outlen, access,
				   ALG_OP_DECRYPT);
}

int32_t kcapi_akcipher_sign(struct kcapi_handle *handle,
			    const uint8_t *in, uint32_t inlen,
			    uint8_t *out, uint32_t outlen, int access)
{
	return _kcapi_cipher_crypt(handle, in, inlen, out, outlen, access,
				   ALG_OP_SIGN);
}

int32_t kcapi_akcipher_verify(struct kcapi_handle *handle,
			      const uint8_t *in, uint32_t inlen,
			      uint8_t *out, uint32_t outlen, int access)
{
	return _kcapi_cipher_crypt(handle, in, inlen, out, outlen, access,
				   ALG_OP_VERIFY);
}

int32_t kcapi_akcipher_stream_init_enc(struct kcapi_handle *handle,
				       struct iovec *iov, uint32_t iovlen)
{
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_ENCRYPT,
				       MSG_MORE);
}

int32_t kcapi_akcipher_stream_init_dec(struct kcapi_handle *handle,
				       struct iovec *iov, uint32_t iovlen)
{
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_DECRYPT,
				       MSG_MORE);
}

int32_t kcapi_akcipher_stream_init_sgn(struct kcapi_handle *handle,
				       struct iovec *iov, uint32_t iovlen)
{
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_SIGN,
				       MSG_MORE);
}

int32_t kcapi_akcipher_stream_init_vfy(struct kcapi_handle *handle,
				       struct iovec *iov, uint32_t iovlen)
{
	return _kcapi_common_send_meta(handle, iov, iovlen, ALG_OP_DECRYPT,
				       MSG_MORE);
}

int32_t kcapi_akcipher_stream_update(struct kcapi_handle *handle,
				     struct iovec *iov, uint32_t iovlen)
{
	/* TODO: vmsplice only works with ALG_MAX_PAGES - 1 -- no clue why */
	if (handle->processed_sg < ALG_MAX_PAGES)
		return _kcapi_common_vmsplice_iov(handle, iov, iovlen,
						  SPLICE_F_MORE);
	else
		return _kcapi_common_send_data(handle, iov, iovlen, MSG_MORE);
}

int32_t kcapi_akcipher_stream_update_last(struct kcapi_handle *handle,
				          struct iovec *iov, uint32_t iovlen)
{
	/* TODO: vmsplice only works with ALG_MAX_PAGES - 1 -- no clue why */
	if (handle->processed_sg < ALG_MAX_PAGES)
		return _kcapi_common_vmsplice_iov(handle, iov, iovlen, 0);
	else
		return _kcapi_common_send_data(handle, iov, iovlen, 0);
}

int32_t kcapi_akcipher_stream_op(struct kcapi_handle *handle,
			         struct iovec *iov, uint32_t iovlen)
{
	if (!iov || !iovlen) {
		kcapi_dolog(LOG_ERR,
			    "Asymmetric operation: No buffer for output data provided");
		return -EINVAL;
	}
	return _kcapi_common_recv_data(handle, iov, iovlen);
}
