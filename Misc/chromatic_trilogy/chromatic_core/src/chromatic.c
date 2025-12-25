/*
 * CHROMATIC CORE V2 - Chromatic Trilogy Challenge 3 (Hardened)
 * 
 * MAJOR CHANGES FROM V1:
 * - Requires unlock key from PATHFINDER (SEED_B)
 * - No edge list displayed
 * - Graph is XOR-encrypted
 * - Must determine chromatic number yourself
 * - Answer is hash of coloring, not coloring itself
 * 
 * Unlock Key: SEED_B (from PATHFINDER flag)
 * Flag: L3m0nCTF{chr0m4t1c_c0mpl3t3_d34db33f}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

// ============== MAGIC MARKER ==============
__attribute__((section(".magic")))
__attribute__((aligned(4096)))
static const char magic_marker[32] = "CHROMATIC_V2_FINAL_HARDENED_";

// ============== CONFIGURATION ==============
#define NUM_NODES 16
#define NUM_COLORS 3

// Obfuscated unlock key (SEED_B XOR 0x13)
static const uint8_t obf_unlock[] = {0x40, 0x56, 0x56, 0x57, 0x4c, 0x51, 0x00};
#define OBFUSCATION_KEY 0x13

// ============== ENCRYPTED EDGE LIST ==============
// Graph is 3-colorable with SCRAMBLED colors:
// Color 0: {0, 4, 7, 10, 13, 15}
// Color 1: {1, 3, 6, 9, 12}
// Color 2: {2, 5, 8, 11, 14}
// Edges only connect different colors
// XOR encrypted with key 0x0A (derived from SEED_B)

__attribute__((section(".edges")))
static const uint8_t encrypted_edges[][2] = {
    {0x0A, 0x01}, // 0-11
    {0x0B, 0x00}, // 1-10
    {0x0B, 0x01}, // 1-11
    {0x08, 0x09}, // 2-3
    {0x08, 0x0E}, // 2-4
    {0x08, 0x0D}, // 2-7
    {0x08, 0x03}, // 2-9
    {0x09, 0x0F}, // 3-5
    {0x09, 0x0D}, // 3-7
    {0x09, 0x02}, // 3-8
    {0x09, 0x00}, // 3-10
    {0x09, 0x04}, // 3-14
    {0x09, 0x05}, // 3-15
    {0x0E, 0x06}, // 4-12
    {0x0F, 0x03}, // 5-9
    {0x0F, 0x07}, // 5-13
    {0x0C, 0x02}, // 6-8
    {0x0C, 0x04}, // 6-14
    {0x0D, 0x06}, // 7-12
    {0x02, 0x07}, // 8-13
    {0x02, 0x05}, // 8-15
    {0x03, 0x01}, // 9-11
    {0x03, 0x04}, // 9-14
    {0x01, 0x06}, // 11-12
    {0x06, 0x04}, // 12-14
    {0x07, 0x04}, // 13-14
    
    // Terminator
    {0xFF, 0xFF}
};

static int edges[64][2];
static int num_edges = 0;
static int graph_decrypted = 0;

// ============== UNLOCK STATE ==============
static int is_unlocked = 0;

// ============== RED HERRING ==============
__attribute__((used))
static const int fake_chromatic_number = 4;  // Wrong value!

// ============== REAL: Unlock verification (obfuscated) ==============
static int verify_unlock(const char* key) {
    // Deobfuscate and compare at runtime
    for (size_t i = 0; obf_unlock[i]; i++) {
        if ((uint8_t)key[i] != (obf_unlock[i] ^ OBFUSCATION_KEY)) {
            return 0;
        }
    }
    return key[6] == '\0';  // Ensure same length
}

// ============== REAL: Derive XOR key ==============
static uint8_t derive_edge_key(const char* unlock) {
    uint8_t key = 0;
    for (size_t i = 0; unlock[i]; i++) {
        key ^= (uint8_t)unlock[i];
    }
    // SEED_B XOR = 'S'^'E'^'E'^'D'^'_'^'B' = 0x3C
    return key;
}

// ============== REAL: Decrypt edges ==============
static void decrypt_edges(const char* unlock) {
    if (graph_decrypted) return;
    
    uint8_t key = derive_edge_key(unlock);
    num_edges = 0;
    
    for (int i = 0; encrypted_edges[i][0] != 0xFF; i++) {
        edges[num_edges][0] = encrypted_edges[i][0] ^ key;
        edges[num_edges][1] = encrypted_edges[i][1] ^ key;
        num_edges++;
    }
    
    graph_decrypted = 1;
}

// ============== VERIFY COLORING ==============
static int verify_coloring(const int* colors) {
    // Check all colors are valid
    for (int i = 0; i < NUM_NODES; i++) {
        if (colors[i] < 0 || colors[i] >= NUM_COLORS) {
            return -1;
        }
    }
    
    // Check no adjacent nodes have same color
    for (int i = 0; i < num_edges; i++) {
        int u = edges[i][0];
        int v = edges[i][1];
        if (colors[u] == colors[v]) {
            return -2;
        }
    }
    
    return 1;
}

// ============== HASH COLORING ==============
// Player must submit hash, not coloring itself
static uint32_t hash_coloring(const int* colors) {
    uint32_t hash = 0x811c9dc5;  // FNV-1a
    for (int i = 0; i < NUM_NODES; i++) {
        hash ^= (uint32_t)colors[i];
        hash *= 0x01000193;
    }
    return hash;
}

// ============== PARSE AND VERIFY ==============
static int check_answer(const char* input) {
    // Input format: hash value in hex (e.g., "a1b2c3d4")
    // OR coloring as comma-separated (e.g., "0,1,2,0,1,2...")
    
    // First try as coloring
    if (strchr(input, ',')) {
        int colors[NUM_NODES];
        char* copy = strdup(input);
        char* tok = strtok(copy, ",");
        int idx = 0;
        
        while (tok && idx < NUM_NODES) {
            colors[idx++] = atoi(tok);
            tok = strtok(NULL, ",");
        }
        free(copy);
        
        if (idx == NUM_NODES && verify_coloring(colors) == 1) {
            return 1;
        }
        return 0;
    }
    
    // Try as hash
    unsigned int provided_hash;
    if (sscanf(input, "%x", &provided_hash) == 1) {
        // Compute expected hash from valid coloring
        // Scrambled: Color 0: {0,4,7,10,13,15}, Color 1: {1,3,6,9,12}, Color 2: {2,5,8,11,14}
        int valid_colors[NUM_NODES] = {0,1,2,1,0,2,1,0,2,1,0,2,1,0,2,0};
        
        uint32_t expected = hash_coloring(valid_colors);
        if (provided_hash == expected) {
            return 1;
        }
    }
    
    return 0;
}

// ============== FLAG DERIVATION ==============
// Key depends on BOTH magic bytes AND correct answer
static char correct_answer[256] = {0};

static uint8_t derive_flag_key(const char* answer) {
    uint8_t buffer[32];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) return 0x42;
    lseek(fd, 0x3000, SEEK_SET);
    read(fd, buffer, 32);
    close(fd);
    
    // XOR magic bytes
    uint8_t key = 0;
    for (int i = 0; i < 32; i++) key ^= buffer[i];
    
    // XOR with answer bytes - makes key answer-dependent!
    for (size_t i = 0; answer[i]; i++) {
        key ^= (uint8_t)answer[i];
    }
    
    return key;
}

// ============== FLAG PRINTING ==============
static void print_flag(void) {
    // Flag: L3m0nCTF{chr0m4t1c_c0mpl3t3_d34db33f}
    // XOR key = magic (0x40) XOR answer (0x2F) = 0x6F
    unsigned char encoded_flag[] = {
        0x23, 0x5c, 0x02, 0x5f, 0x01, 0x2c, 0x3b, 0x29,
        0x14, 0x0c, 0x07, 0x1d, 0x5f, 0x02, 0x5b, 0x1b,
        0x5e, 0x0c, 0x30, 0x0c, 0x5f, 0x02, 0x1f, 0x03,
        0x5c, 0x1b, 0x5c, 0x30, 0x0b, 0x5c, 0x5b, 0x0b,
        0x0d, 0x5c, 0x5c, 0x09, 0x12, 0x00
    };
    
    uint8_t key = derive_flag_key(correct_answer);
    
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
    printf("        CHROMATIC CORE - CHALLENGE COMPLETE         \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" You found a valid coloring for the hidden graph!   \n");
    printf(" The chromatic number χ(G) = 3.                     \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" FLAG: ");
    for (size_t i = 0; encoded_flag[i]; i++) {
        printf("%c", encoded_flag[i] ^ key);
    }
    printf("\n\n");
    printf(" CONGRATULATIONS! You have completed the trilogy!   \n");
    printf("════════════════════════════════════════════════════\n");
}

// ============== UI: Locked ==============
static void display_locked(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║               CHROMATIC CORE                       ║\n");
    printf("║            Chromatic Trilogy - Part 3              ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║ This challenge is LOCKED.                          ║\n");
    printf("║                                                    ║\n");
    printf("║ Enter the unlock key from the previous challenge.  ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============== UI: Unlocked ==============
static void display_unlocked(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║               CHROMATIC CORE                       ║\n");
    printf("║            Chromatic Trilogy - Part 3              ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║ Challenge UNLOCKED.                                ║\n");
    printf("║                                                    ║\n");
    printf("║ A graph is hidden in this binary.                  ║\n");
    printf("║ Find it. Determine its chromatic number.           ║\n");
    printf("║ Provide a valid coloring.                          ║\n");
    printf("║                                                    ║\n");
    printf("║ The graph has %d nodes.                            ║\n", NUM_NODES);
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============== HIDDEN DEBUG ==============
__attribute__((used))
void debug_edges(void) {
    printf("Edges (%d):\n", num_edges);
    for (int i = 0; i < num_edges; i++) {
        printf("(%d,%d) ", edges[i][0], edges[i][1]);
    }
    printf("\n");
}

// ============== MAIN ==============
int main(int argc, char** argv) {
    if (argc < 2) {
        display_locked();
        printf("Usage: %s <unlock_key> [coloring]\n", argv[0]);
        return 1;
    }
    
    if (!verify_unlock(argv[1])) {
        display_locked();
        printf("Invalid unlock key.\n");
        return 1;
    }
    
    decrypt_edges(argv[1]);
    is_unlocked = 1;
    
    if (argc >= 3) {
        if (check_answer(argv[2])) {
            strncpy(correct_answer, argv[2], 255);
            display_unlocked();
            print_flag();
            return 0;
        } else {
            display_unlocked();
            printf("Invalid coloring.\n");
            return 1;
        }
    }
    
    display_unlocked();
    
    printf("Enter coloring (comma-separated, %d values): ", NUM_NODES);
    fflush(stdout);
    
    char input[256];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        
        if (strlen(input) > 0) {
            if (check_answer(input)) {
                strncpy(correct_answer, input, 255);
                print_flag();
                return 0;
            } else {
                printf("Invalid coloring.\n");
            }
        }
        
        printf("Enter coloring: ");
        fflush(stdout);
    }
    
    return 0;
}
