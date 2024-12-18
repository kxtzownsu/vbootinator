#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "arg_checks.h"

#define VB2_SECDATA_FWMP_VERSION 0x10  /* 1.0 */
#define VB2_SECDATA_FWMP_HASH_SIZE 32  /* enough for SHA-256 */

struct vb2_secdata_fwmp {
    /* CRC-8 of fields following struct_size */
    uint8_t crc8;

    /* Structure size in bytes */
    uint8_t struct_size;

    /* Structure version (4 bits major, 4 bits minor) */
    uint8_t struct_version;

    uint8_t reserved0;

    /* Flags; see enum vb2_secdata_fwmp_flags */
    uint32_t flags;

    /* Hash of developer kernel key */
    uint8_t dev_key_hash[VB2_SECDATA_FWMP_HASH_SIZE];
};

/* Flags for firmware management parameters */
enum vb2_secdata_fwmp_flags {
    VB2_SECDATA_FWMP_DEV_DISABLE_BOOT = (1 << 0),
    VB2_SECDATA_FWMP_DEV_DISABLE_RECOVERY = (1 << 1),
    VB2_SECDATA_FWMP_DEV_ENABLE_EXTERNAL = (1 << 2),
    VB2_SECDATA_FWMP_DEV_ENABLE_ALTFW = (1 << 3),
    VB2_SECDATA_FWMP_DEV_ENABLE_OFFICIAL_ONLY = (1 << 4),
    VB2_SECDATA_FWMP_DEV_USE_KEY_HASH = (1 << 5),
    VB2_SECDATA_FWMP_DEV_DISABLE_CCD_UNLOCK = (1 << 6),
    VB2_SECDATA_FWMP_DEV_FIPS_MODE = (1 << 7),
};

uint8_t vb2_crc8(const void *vptr, uint32_t size) {
    const uint8_t *data = vptr;
    unsigned crc = 0;
    uint32_t i, j;

    for (j = size; j; j--, data++) {
        crc ^= (*data << 8);
        for (i = 8; i; i--) {
            if (crc & 0x8000)
                crc ^= (0x1070 << 3);
            crc <<= 1;
        }
    }

    return (uint8_t)(crc >> 8);
}

static inline uint32_t vb2_secdata_fwmp_crc(struct vb2_secdata_fwmp *sec) {
    int version_offset = offsetof(struct vb2_secdata_fwmp, struct_version);
    return vb2_crc8((void *)sec + version_offset,
                    sec->struct_size - version_offset);
}

void list_flags() {
    printf("Available Flags:\n");
    printf("0x1  - VB2_SECDATA_FWMP_DEV_DISABLE_BOOT\n");
    printf("0x2  - VB2_SECDATA_FWMP_DEV_DISABLE_RECOVERY\n");
    printf("0x4  - VB2_SECDATA_FWMP_DEV_ENABLE_EXTERNAL\n");
    printf("0x8  - VB2_SECDATA_FWMP_DEV_ENABLE_ALTFW\n");
    printf("0x10 - VB2_SECDATA_FWMP_DEV_ENABLE_OFFICIAL_ONLY\n");
    printf("0x20 - VB2_SECDATA_FWMP_DEV_USE_KEY_HASH\n");
    printf("0x40 - VB2_SECDATA_FWMP_DEV_DISABLE_CCD_UNLOCK\n");
    printf("0x80 - VB2_SECDATA_FWMP_DEV_FIPS_MODE\n");
}

int main(int argc, char *argv[]) {
    gargc = argc;
    gargv = argv;

    if (fbool("--help") || argc == 1) {
        printf("USAGE: %s <optl. flags>\n", argv[0]);
        printf("e.g: %s --flags=0x1 --raw\n", argv[0]);
        printf("-=-=-=-=-=-=-\n");
        printf("--raw - prints the output as raw hex bytes\n");
        printf("--tpm - value to flash to a TPM chip, same as --raw, but prints like '00 01' instead of raw hex\n");
        printf("--flags - sets what flags to use for fwmp (requires --tpm)\n");
        printf("--list-flags - list flags\n");
        printf("--combine-flags - combines flags, e.g: --combine-flags=0x1,0x2,0x3\n");
        printf("--dev-hash - what developer hash to use in FWMP\n");
        printf("--help - shows this message :3\n");
        printf("-=-=-=-=-=-=-\n");
        printf("%s was created by kxtzownsu\n", argv[0]);
        printf("Credits go to Hannah for making the arg parsing system\n");
        exit(0);
    }

    if (fbool("--list-flags")) {
        list_flags();
        exit(0);
    }

    struct vb2_secdata_fwmp secdata;
    memset(&secdata, 0, sizeof(secdata));
    secdata.struct_size = sizeof(struct vb2_secdata_fwmp);
    secdata.struct_version = VB2_SECDATA_FWMP_VERSION;

    if (fbool("--tpm") || fbool("--raw")) {
        if (fequals("--flags")) {
            secdata.flags = strtol(fequals("--flags"), NULL, 0);
        }
    } else if (fbool("--flags")) {
        fprintf(stderr, "Error: --flags requires --tpm.\n");
        return 1;
    }

    if (fequals("--combine-flags")) {
        char *combine_values = fequals("--combine-flags");
        char *token = strtok(combine_values, ",");
        while (token != NULL) {
            secdata.flags |= strtol(token, NULL, 0);
            token = strtok(NULL, ",");
        }
    }

    if (fequals("--dev-hash")) {
        char *hash = fequals("--dev-hash");
        size_t len = strlen(hash) > VB2_SECDATA_FWMP_HASH_SIZE
                         ? VB2_SECDATA_FWMP_HASH_SIZE
                         : strlen(hash);
        memcpy(secdata.dev_key_hash, hash, len);
    }

    secdata.crc8 = vb2_secdata_fwmp_crc(&secdata);

    if (fbool("--raw")) {
        fwrite(&secdata, sizeof(secdata), 1, stdout);
    } else if (fbool("--tpm")) {
        for (size_t i = 0; i < sizeof(secdata); i++) {
            printf("%02X ", ((uint8_t *)&secdata)[i]);
        }
        printf("\n");
    } else {
        printf("0x%X\n", secdata.flags);
    }

    return 0;
}
