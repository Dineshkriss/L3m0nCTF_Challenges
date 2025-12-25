#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

// ============== RED HERRING: Fake AES constants ==============
static const uint32_t fake_aes_sbox[] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76
};

static const char fake_encrypted_data[] = {
    0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe
};

// ============== RED HERRING: Unused decryption function ==============
void fake_decrypt_payload(uint8_t *data, size_t len) {
    // This function does nothing useful but looks suspicious
    for (size_t i = 0; i < len; i++) {
        data[i] ^= fake_aes_sbox[i % 16];
        data[i] = (data[i] << 3) | (data[i] >> 5);
    }
}

// ============== RED HERRING: Decoy environment variables ==============
static const char* decoy_env_vars[] = {
    "LD_PRELOAD",
    "LD_AUDIT",
    "SYSTEM_KEY",
    "AUTH_TOKEN",
    NULL
};

// ============== TARGET HASH (XOR-obfuscated) ==============
// This is the REAL check - contestants must find this
static uint8_t xor_key[] = {0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF, 0x00};
static uint8_t encoded_hash[] = {0x8d, 0xed, 0x2d, 0xe0, 0x20, 0x15, 0x28, 0xe0};

// ============== RED HERRING: Fake backdoor function ==============
void fake_backdoor_function() {
    // This looks like a backdoor but never gets called
    // Uses a DIFFERENT encoding to mislead (static XOR 0x42)
    unsigned char fake_flag[] = {
        0x24, 0x2c, 0x03, 0x27, 0x7b, 0x24, 0x03, 0x2e, 0x25, 0x61, 0x24,
        0x2c, 0x03, 0x27, 0x61, 0x2d, 0x2b, 0x21, 0x25, 0x61, 0x36, 0x30,
        0x39, 0x7d, 0x00
    };
    // Fake uses simple XOR (misleading)
    for (int i = 0; fake_flag[i]; i++) printf("%c", fake_flag[i] ^ 0x42);
    printf("\n");
    system("/bin/sh");  // Decoy
}

// ============== REAL FUNCTIONS ==============
void normal_function() {
    printf("[*] Monitoring system: nominal\n");
    printf("[*] All security checks passed\n");
}

// ============== DYNAMIC KEY DERIVATION ==============
static uint8_t derive_xor_key() {
    // Derive XOR key from the magic bytes at offset 0x3000
    // This makes the flag encoding context-dependent and harder to reverse
    uint8_t buffer[6];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) return 0x42;  // Fallback to default
    
    lseek(fd, 0x3000, SEEK_SET);
    ssize_t n = read(fd, buffer, 6);
    close(fd);
    
    if (n < 6) return 0x42;  // Fallback
    
    // XOR all bytes of "CHROMA" together
    uint8_t key = 0;
    for (int i = 0; i < 6; i++) {
        key ^= buffer[i];
    }
    
    return key;  // Result: 'C'^'H'^'R'^'O'^'M'^'A' = 0x1A
}

void backdoor_function() {
    // The real backdoor - prints the actual flag
    // Flag is XOR-encoded with DYNAMICALLY DERIVED KEY (from magic bytes)
    // Key = XOR of all bytes in "CHROMA" at offset 0x3000 = 0x1A
    unsigned char encoded_flag[] = {
        0x56, 0x29, 0x77, 0x2a, 0x74, 0x59, 0x4e, 0x5c, 0x61, 0x6a, 0x72,
        0x2e, 0x74, 0x6e, 0x2a, 0x77, 0x45, 0x68, 0x29, 0x69, 0x2a, 0x76,
        0x6c, 0x29, 0x68, 0x45, 0x2b, 0x7c, 0x6f, 0x74, 0x79, 0x45, 0x77,
        0x2e, 0x69, 0x6e, 0x29, 0x68, 0x63, 0x67, 0x00
    };
    
    // Derive key from binary's magic section
    uint8_t key = derive_xor_key();
    
    // Decode and print
    for (int i = 0; encoded_flag[i]; i++) {
        printf("%c", encoded_flag[i] ^ key);
    }
    printf("\n");
}

// ============== SIMPLE HASH (FNV-1a style) ==============
static uint64_t compute_hash(uint8_t *data, size_t len) {
    uint64_t hash = 0xcbf29ce484222325ULL;  // FNV offset basis
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x100000001b3ULL;  // FNV prime
    }
    return hash;
}

// ============== RED HERRING: Fake hash check ==============
static int fake_check_environment() {
    // Check for decoy environment variables (always returns 1)
    for (int i = 0; decoy_env_vars[i]; i++) {
        if (getenv(decoy_env_vars[i])) {
            return 0;  // Pretend to fail
        }
    }
    return 1;
}

// ============== RED HERRING: Anti-debug that doesn't work ==============
static int fake_anti_debug() {
    // Looks like anti-debug but is ineffective
    char buf[1024];
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd >= 0) {
        read(fd, buf, sizeof(buf));
        close(fd);
        // Fake check that always passes
        return 1;
    }
    return 1;
}

// ============== REAL CHECK: Validate calling binary ==============
static int validate_caller() {
    // Simplified: Just check that we can read the binary
    // The hash check is too fragile for this challenge format
    // (patching changes the hash)
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) return 0;
    close(fd);
    return 1;  // Binary exists and is readable
}

// ============== REAL CHECK: Verify magic bytes in binary ==============
static int verify_magic_bytes() {
    // Checks: At offset 0x1000 in the binary, bytes should be "CHROMA"
    uint8_t buffer[16];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) return 0;
    
    lseek(fd, 0x3000, SEEK_SET);  // Offset where magic should be
    ssize_t n = read(fd, buffer, 6);
    close(fd);
    
    if (n < 6) return 0;
    
    uint8_t expected[] = {0x01, 0x0a, 0x10, 0x0d, 0x0f, 0x03};
    for (int i = 0; i < 6; i++) expected[i] ^= 0x42;
    
    return memcmp(buffer, expected, 6) == 0;
}

// ============== REAL CHECK: Command line argument ==============
static int check_cmdline() {
    char cmdline[1024] = {0};
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0) return 0;
    ssize_t n = read(fd, cmdline, sizeof(cmdline) - 1);
    close(fd);
    
    if (n <= 0) return 0;
    
    // /proc/self/cmdline uses NULL bytes as separators between args
    // Replace them with spaces so strstr() can search the whole string
    for (ssize_t i = 0; i < n - 1; i++) {
        if (cmdline[i] == '\0') cmdline[i] = ' ';
    }
    
    // Must have "--daemon" flag (like sshd's -D)
    return strstr(cmdline, "--daemon") != NULL;
}

// ============== RED HERRING: Fake resolver that looks important ==============
static void* decoy_resolve_internal(void) {
    // This function exists to mislead - never actually used
    if (fake_check_environment()) {
        if (fake_anti_debug()) {
            fake_decrypt_payload((uint8_t*)fake_encrypted_data, 8);
            return (void*)fake_backdoor_function;
        }
    }
    return (void*)normal_function;
}

// ============== THE REAL RESOLVER (runs before main) ==============
void (*resolve_function(void))(void) {
    
    // XZ-style: Multiple validation layers
    
    // RED HERRING: Call fake checks (results ignored)
    fake_check_environment();
    fake_anti_debug();
    
    // Layer 1: Is this the right binary? (Hash check)
    if (!validate_caller()) {
        return normal_function;
    }
    
    // Layer 2: Does binary have our magic marker?
    if (!verify_magic_bytes()) {
        return normal_function;
    }
    
    // Layer 3: Running in correct mode?
    if (!check_cmdline()) {
        return normal_function;
    }
    
    // All checks passed - we're in the target process!
    return backdoor_function;
}

// ============== IFUNC DECLARATION ==============
void system_check() __attribute__((ifunc("resolve_function")));

// ============== RED HERRING: Exported functions that look suspicious ==============
__attribute__((visibility("default")))
void init_security_module(void) {
    // Does nothing, just a distraction
    printf("[*] Security module initialized\n");
}

__attribute__((visibility("default")))
int verify_credentials(const char* user, const char* pass) {
    // Fake function that always returns 1
    return 1;
}
