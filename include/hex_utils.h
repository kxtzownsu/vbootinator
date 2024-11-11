uint32_t convert_to_uint32(const char *str) {
    char *endptr;
    unsigned long ul_value = strtoul(str, &endptr, 0);
    
    if (strlen(str) > 10){
        fprintf(stderr, "The entered kernver, %s, was longer than 10 characters (including 0x),\nplease refine the input and try again.", str);
        exit(1);
    }

    return (uint32_t)ul_value;
}

bool is_valid_hex(const char *str) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        str += 2;
    }

    return *str != '\0' && strspn(str, "0123456789abcdefABCDEF") == strlen(str);
}

void print_hex(const uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        printf("%02x ", data[i]);
    }
}