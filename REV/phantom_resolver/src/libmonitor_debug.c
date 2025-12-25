// Debug version with printf to see what's failing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t xor_key[] = {0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF, 0x00};
static uint8_t encoded_hash[] = {0x8d, 0xed, 0x2d, 0xe0, 0x20, 0x15, 0x28, 0xe0};

void normal_function() {
    printf("[DBG] Called: normal_function\\n");
    printf("[*] Monitoring system: nominal\\n");
}

void backdoor_function() {
    printf("[DBG] Called: backdoor_function\\n");
    printf("flag{ph4nt0m_r3s0lv3r_1func_m4st3ry}\\n");
}

static uint64_t compute_hash(uint8_t *data, size_t len) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

static int validate_caller() {
    uint8_t buffer[4096];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) {
        printf("[DBG] validate_caller: Could not open /proc/self/exe\\n");
        return 0;
    }
    
    ssize_t n = read(fd, buffer, sizeof(buffer));
    close(fd);
    if (n < 4096) {
        printf("[DBG] validate_caller: Could not read 4096 bytes (got %zd)\\n", n);
        return 0;
    }
    
    uint64_t caller_hash = compute_hash(buffer, 4096);
    uint64_t expected = 0;
    for (int i = 0; i < 8; i++) {
        expected |= ((uint64_t)(encoded_hash[i] ^ xor_key[i])) << (i * 8);
    }
    
    printf("[DBG] validate_caller: hash=0x%016llx expected=0x%016llx match=%d\\n",
           (unsigned long long)caller_hash, (unsigned long long)expected, caller_hash == expected);
    
    return caller_hash == expected;
}

static int verify_magic_bytes() {
    uint8_t buffer[16];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) {
        printf("[DBG] verify_magic_bytes: Could not open /proc/self/exe\\n");
        return 0;
    }
    
    lseek(fd, 0x3000, SEEK_SET);
    ssize_t n = read(fd, buffer, 6);
    close(fd);
    
    if (n < 6) {
        printf("[DBG] verify_magic_bytes: Could not read 6 bytes\\n");
        return 0;
    }
    
    uint8_t expected[] = {0x01, 0x0a, 0x10, 0x0d, 0x0f, 0x03};
    for (int i = 0; i < 6; i++) expected[i] ^= 0x42;
    
    printf("[DBG] verify_magic_bytes: found='%c%c%c%c%c%c' expected='CHROMA' match=%d\\n",
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
           memcmp(buffer, expected, 6) == 0);
    
    return memcmp(buffer, expected, 6) == 0;
}

static int check_cmdline() {
    char cmdline[1024] = {0};
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0) {
        printf("[DBG] check_cmdline: Could not open /proc/self/cmdline\\n");
        return 0;
    }
    ssize_t n = read(fd, cmdline, sizeof(cmdline) - 1);
    close(fd);
    
    if (n < 0) {
        printf("[DBG] check_cmdline: Could not read\\n");
        return 0;
    }
    
    int has_daemon = strstr(cmdline, "--daemon") != NULL;
    printf("[DBG] check_cmdline: has_daemon=%d\\n", has_daemon);
    
    return has_daemon;
}

void (*resolve_function_debug(void))(void) {
    printf("[DBG] === IFUNC RESOLVER CALLED ===\\n");
    
    int check1 = validate_caller();
    int check2 = verify_magic_bytes();
    int check3 = check_cmdline();
    
    printf("[DBG] Results: hash=%d magic=%d cmdline=%d\\n", check1, check2, check3);
    
    if (check1 && check2 && check3) {
        printf("[DBG] === ALL CHECKS PASSED - BACKDOOR ===\\n");
        return backdoor_function;
    }
    
    printf("[DBG] ===CHECKS FAILED - NORMAL ===\\n");
    return normal_function;
}

void system_check_debug() __attribute__((ifunc("resolve_function_debug")));
