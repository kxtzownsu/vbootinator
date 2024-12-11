#include <stdio.h>
#include <stddef.h>
#include <stdint.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "arg_checks.h"
#include "hex_utils.h"

uint32_t fwarg;

#define VB2_SHA256_DIGEST_SIZE 32
#define VB2_SECDATA_FIRMWARE_VERSION 2

struct vb2_secdata_firmware {
	uint8_t struct_version;
	uint8_t flags;
	uint32_t fwver;
	uint8_t reserved[3];
	uint8_t crc8;
} __attribute__((packed));

/* Flags for firmware space */
enum vb2_secdata_firmware_flags {
	/*
	 * Last boot was developer mode.  TPM ownership is cleared when
	 * transitioning to/from developer mode.  Set/cleared by
	 * vb2_check_dev_switch().
	 */
	VB2_SECDATA_FIRMWARE_FLAG_LAST_BOOT_DEVELOPER = (1 << 0),

	/*
	 * Virtual developer mode switch is on.  Set/cleared by the
	 * keyboard-controlled dev screens in recovery mode.  Cleared by
	 * vb2_check_dev_switch().
	 */
	VB2_SECDATA_FIRMWARE_FLAG_DEV_MODE = (1 << 1),
};



struct vb2_context {
    void *secdata_firmware;
};

uint8_t vb2_crc8(const void *vptr, uint32_t size)
{
    const uint8_t *data = vptr;
    unsigned crc = 0;
    uint32_t i, j;

    for (j = size; j; j--, data++) {
        crc ^= (*data << 8);
        for(i = 8; i; i--) {
            if (crc & 0x8000)
                crc ^= (0x1070 << 3);
            crc <<= 1;
        }
    }

    return (uint8_t)(crc >> 8);
}

static uint8_t secdata_firmware_crc(struct vb2_context *ctx)
{
    size_t offset, size;

    struct vb2_secdata_firmware *sec
        = (void *)ctx->secdata_firmware;
    offset = 0;
    size = sizeof(struct vb2_secdata_firmware) - sizeof(uint8_t); // Excluding the CRC field

    return vb2_crc8(ctx->secdata_firmware + offset, size);
}

int main(int argc, char *argv[]) {
    gargc = argc;
    gargv = argv;

    if (fbool("--help") || argc == 1) {
        printf("USAGE: %s <fw_versions> <optl. flags>\n", argv[0]);
        printf("e.g: %s 0x00010001 --raw\n", argv[0]);
        printf("-=-=-=-=-=-=-\n");
        printf("--raw - prints the output as raw hex bytes\n");
        printf("--developer-mode - sets the boot mode to developer mode\n");
        printf("--help - shows this message :3\n");
        printf("-=-=-=-=-=-=-\n");
        printf("KVG was created by kxtzownsu\n");
        printf("Credits go to Hannah for making the arg parsing system\n");
        exit(0);
    }

    if (is_valid_hex(argv[1])) {
        fwarg = convert_to_uint32(argv[1]);
    } else {
        printf("The entered fwver: %s, wasn't detected as valid hexadecimal, please try again.\n", argv[1]);
        exit(1);
    }

    struct vb2_secdata_firmware secdata;
    secdata.struct_version = 0x02;
    if (fbool("--developer-mode")) {
        secdata.flags |= VB2_SECDATA_FIRMWARE_FLAG_LAST_BOOT_DEVELOPER;
        secdata.flags |= VB2_SECDATA_FIRMWARE_FLAG_DEV_MODE;
    } else {
        secdata.flags = 0x0;
    }
    secdata.fwver = fwarg; 
    memset(secdata.reserved, 0, sizeof(secdata.reserved));

    struct vb2_context ctx;
    ctx.secdata_firmware = (void *)&secdata;

    secdata.crc8 = secdata_firmware_crc(&ctx);
    
    
    if (fbool("--raw")) {
        fwrite(&secdata, sizeof(secdata), 1, stdout);
    } else {
        print_hex((uint8_t *)&secdata, sizeof(struct vb2_secdata_firmware));          
    }

    return 0;
}
