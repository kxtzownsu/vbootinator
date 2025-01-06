# GBB flag-inator

Select and decode values for Depthcharge/Vboot/TPM values

## Website
[https://vbootinator.havenoverflow.dev](https://vbootinator.havenoverflow.dev)

## Directory Guide

```
web/ - web version of Vbootinator
src/ - the C source files for Vbootinator
include/ - any `.h` files we need, e.g: `arg_checks.h`
build/ - where compiled binaries will go
doc/ - documentation, mostly just building info and FAQs
```

## Compiling / Building
See docs/BUILDING.md

## Installation
Run `sudo make install`, `sudo` can be swapped out with `doas` if needed. It can be removed if you are already `root` or UID 0.

## Credits: 
- kxtzownsu - writing vbootinator web & C
- OlyB - This project wouldn't exist without him. He wrote all of GBB Flag-inator by himself. Major credits to him.
- HavenOverflow - mental support & allowing me to release under their name
- Chromium Code Search - not making me have to download the vboot_reference source code to get the structs for everything