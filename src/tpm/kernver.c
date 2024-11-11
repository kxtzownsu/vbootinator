/*
This code is from the KVS repo,
specifically the branch 'v2'.
https://github.com/kxtzownsu/KVS
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "arg_checks.h"
#include "hex_utils.h"

uint32_t kvarg;

// basically almost all of this code was pieced together
// using vboot_reference code, credits to Google
// for writing most of this code in a sense :3

#define VB2_SHA256_DIGEST_SIZE 32

struct vb2_secdata_kernel_v0 {
    uint8_t struct_version;  
    uint32_t uid;
    uint32_t kernver;
    uint8_t reserved[3];
    uint8_t crc8;
} __attribute__((packed));

struct vb2_secdata_kernel_v1 {
	uint8_t struct_version;  /* 1.0 (or 0x10 in v0 format) */
	uint8_t struct_size;
	uint8_t crc8;
	uint8_t flags;
	uint32_t kernver;
	uint8_t ec_hash[VB2_SHA256_DIGEST_SIZE];
};


struct vb2_secdata_kernel_v0 secdata0;
struct vb2_secdata_kernel_v1 secdata1;


struct vb2_context {
    void *secdata_kernel;
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

#define MAJOR_VER(x) (((x) & 0xf0) >> 4)
#define MINOR_VER(x) ((x) & 0x0f)

static inline int is_v0(struct vb2_context *ctx)
{
	struct vb2_secdata_kernel_v1 *sec = (void *)ctx->secdata_kernel;
	return MAJOR_VER(sec->struct_version) == 0;
}

static uint8_t secdata_kernel_crc(struct vb2_context *ctx)
{
	size_t offset, size;

	if (is_v0(ctx)) {
		offset = 0;
		size = offsetof(struct vb2_secdata_kernel_v0, crc8);
	} else {
		struct vb2_secdata_kernel_v1 *sec
			= (void *)ctx->secdata_kernel;
		offset = offsetof(struct vb2_secdata_kernel_v1, flags);
		size = sec->struct_size - offset;
	}

	return vb2_crc8(ctx->secdata_kernel + offset, size);
}


int main(int argc, char *argv[]) {
    gargc = argc;
	gargv = argv;
    char *version = fequals("--ver");

    // if --help or no args are passsed
    // print the usage and an example command
    if (fbool("--help") || argc == 1){
        printf("USAGE: %s <kernver> <optl. flags>\n", argv[0]);
        printf("e.g: %s 0x00010001 --raw\n", argv[0]);
        printf("-=-=-=-=-=-=-\n");
        printf("--raw - prints the output as raw hex bytes\n");
        printf("--ver=<0/1> - specifies the kernver struct version to use, oldui boards use ver0 while newui boards use ver1\n");
        printf("--help - shows this message :3\n");
        printf("-=-=-=-=-=-=-\n");
        printf("KVG was created by kxtzownsu\n");
        printf("Credits go to Hannah for making the arg parsing system\n");
        exit(0);
    }

    // make sure the user sends us a correct hex value, 
    // we dont want to just blindly trust that its correct
    if (is_valid_hex(argv[1])) {
        kvarg = convert_to_uint32(argv[1]);
    } else {
        printf("The entered kernver: %s, wasn't detected as valid hexadecimal, please try again.\n", argv[1]);
        exit(1);
    }

    if (strcmp(version, "0") != 0 && strcmp(version, "1") != 0) {
        printf("The entered struct version: %s, wasn't a valid option (see --help). Please try again.\n", version);
        exit(1);
    }

    if (!strcmp(version, "0")){
        secdata0.struct_version = 0x02;
        secdata0.uid = 0x4752574c;
        secdata0.reserved[0] = 0x00;
        secdata0.reserved[1] = 0x00;
        secdata0.reserved[2] = 0x00;
        secdata0.kernver = kvarg;

        struct vb2_context ctx;
        ctx.secdata_kernel = (void *)&secdata0;

        secdata0.crc8 = secdata_kernel_crc(&ctx);
    }

    if (!strcmp(version, "1")) {
        secdata1.struct_version = 0x10;
        secdata1.struct_size = sizeof(struct vb2_secdata_kernel_v1);
	    secdata1.flags = 0x0;
	    secdata1.kernver = kvarg;

        struct vb2_context ctx;
        ctx.secdata_kernel = (void *)&secdata1;
        secdata1.crc8 = secdata_kernel_crc(&ctx);
    }
    
    // god i hate this nesting, TODO: find a better way to detect if we're specifying ver0 or ver1
    if (fbool("--raw")) {
        if (!strcmp(version, "0")) {
            fwrite(&secdata0, sizeof(secdata0), 1, stdout);
        }
	    if (!strcmp(version, "1")) {
	        fwrite(&secdata1, sizeof(secdata1), 1, stdout);
	    }
    } else {
        if (!strcmp(version, "0")) {
            print_hex((uint8_t *)&secdata0, sizeof(struct vb2_secdata_kernel_v0));          
        }
	    if (!strcmp(version, "1")) {
	        print_hex((uint8_t *)&secdata1, sizeof(struct vb2_secdata_kernel_v1));
	    }
    }

    return 0;
}