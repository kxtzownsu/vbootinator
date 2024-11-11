#######################
# build targets below #
#######################
# all (default)
# install
# gbb
# ┗ flags
# ┗ keys
# tpm
# ┗ fwmp
# ┗ fwver
# ┗ kernver
#######################
# To build a subsection like `flags`,
# it would just be this: `make flags`
#######################

CC ?= gcc
SHELL ?= /bin/sh
CFLAGS := \
		-g \
		-Werror \
		-Iinclude \
		-O3 \
		-static

$(shell mkdir -p build/bin)
$(shell mkdir -p build/lib)
$(shell mkdir -p build/share/vbootinator)


all: gbb tpm

gbb: flags keys
tpm: fwmp fwver kernver

# GBB stuff like flags and keys. 
flags:
	$(CC) src/gbb/flags.c -o build/bin/vbt-gbbflag $(CFLAGS)

# All the scripts & binaries that are needed for vbt-gbbkey are in build/usr/, or in /usr/local/
keys:
	$(CC) src/gbb/keys.c -o build/bin/vbt-gbbkey $(CFLAGS)


# TPM stuff like fwmp and kernver
fwmp: 
	$(CC) src/tpm/fwmp.c -o build/bin/vbt-tpmfwmp $(CFLAGS)

fwver:
	$(CC) src/tpm/fwver.c -o build/bin/vbt-tpmfwver $(CFLAGS)

kernver:
	$(CC) src/tpm/kernver.c -o build/bin/vbt-tpmkernver $(CFLAGS)