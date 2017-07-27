#ifndef OBFUSE_H
#define OBFUSE_H

uint32_t ca_efuse_programmer(
    uint8_t key_seed[16],
    uint8_t key[16],
    uint8_t key1[16],
    uint8_t key2[16],
    uint8_t key3[16],
    uint8_t rsa_key[256]);

uint32_t ca_get_secure_bit(void);

uint32_t ca_key_verify(uint8_t *data_in, size_t size);

uint32_t ca_set_tee_os_version(void);

uint32_t ca_get_tee_os_version(void);

#endif
