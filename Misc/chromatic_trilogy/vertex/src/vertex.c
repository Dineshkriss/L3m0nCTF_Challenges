/*
 * VERTEX V2 - Chromatic Trilogy Challenge 1 (Hardened)
 * 
 * MAJOR CHANGES FROM V1:
 * - No graph information displayed
 * - Adjacency matrix is XOR-encrypted in .data
 * - Must reverse engineer decryption logic
 * - Flag contains SEED_A needed for PATHFINDER
 * 
 * Flag: L3m0nCTF{v3rt3x_GRAPHKEY_7f2a9b}
 *       The GRAPHKEY portion is used to unlock PATHFINDER
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

// ============== MAGIC MARKER FOR KEY DERIVATION ==============
__attribute__((section(".magic")))
__attribute__((aligned(4096)))
static const char magic_marker[32] = "VERTEX_V2_CHROMATIC_HARDENED";

// ============== CONFIGURATION ==============
#define NUM_VERTICES 12
#define GRAPH_XOR_KEY 0x5A  // Key to decrypt adjacency matrix

// ============== ENCRYPTED ADJACENCY MATRIX ==============
// The actual graph is bipartite with SCRAMBLED sets:
// Set A = {0, 2, 5, 7, 8, 11}  (NOT simple 0-5!)
// Set B = {1, 3, 4, 6, 9, 10}
// XOR key = 0x76 (derived from magic_marker first 16 bytes)
__attribute__((section(".graph_data")))
static const uint8_t encrypted_adj[NUM_VERTICES][NUM_VERTICES] = {
    // Row 0: connects to [1, 9, 10]
    {0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76},
    // Row 1: connects to [0, 5, 7, 8, 11]
    {0x77, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x77, 0x77, 0x76, 0x76, 0x77},
    // Row 2: connects to [3, 4, 10]
    {0x76, 0x76, 0x76, 0x77, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76},
    // Row 3: connects to [2, 5, 11]
    {0x76, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77},
    // Row 4: connects to [2]
    {0x76, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76},
    // Row 5: connects to [1, 3, 10]
    {0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76},
    // Row 6: connects to [8]
    {0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x76, 0x76},
    // Row 7: connects to [1, 9, 10]
    {0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76},
    // Row 8: connects to [1, 6, 9]
    {0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x76},
    // Row 9: connects to [0, 7, 8]
    {0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76, 0x76, 0x76},
    // Row 10: connects to [0, 2, 5, 7, 11]
    {0x77, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x77},
    // Row 11: connects to [1, 3, 10]
    {0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76}
};

// ============== DECRYPTED GRAPH (computed at runtime) ==============
static uint8_t adj_matrix[NUM_VERTICES][NUM_VERTICES];
static int graph_decrypted = 0;

// ============== RED HERRING: Fake decryption key ==============
__attribute__((section(".rodata")))
static const uint8_t fake_key[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};

// ============== RED HERRING: Suspicious function ==============
__attribute__((used))
void suspicious_decrypt(uint8_t* data, size_t len) {
    // This is a red herring - uses wrong key
    for (size_t i = 0; i < len; i++) {
        data[i] ^= fake_key[i % 6];
    }
}

// ============== REAL: Graph decryption (obfuscated) ==============
static void init_graph(void) {
    if (graph_decrypted) return;
    
    uint8_t key = 0;
    for (int i = 0; i < 16; i++) {
        key ^= (uint8_t)magic_marker[i];
    }
    
    for (int i = 0; i < NUM_VERTICES; i++) {
        for (int j = 0; j < NUM_VERTICES; j++) {
            adj_matrix[i][j] = encrypted_adj[i][j] ^ key;
        }
    }
    
    graph_decrypted = 1;
}

// ============== REAL: Key derivation for flag ==============
// Key depends on BOTH magic bytes AND correct answer
static uint8_t derive_flag_key(const char* answer) {
    uint8_t buffer[32];
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0) return 0x42;
    
    lseek(fd, 0x3000, SEEK_SET);
    ssize_t n = read(fd, buffer, 32);
    close(fd);
    
    if (n < 32) return 0x42;
    
    uint8_t key = 0;
    for (int i = 0; i < 32; i++) {
        key ^= buffer[i];
    }
    
    for (size_t i = 0; answer[i]; i++) {
        key ^= (uint8_t)answer[i];
    }
    
    return key;
}

// ============== ANSWER VERIFICATION ==============
// Player must provide actual bipartite partition
// Format: "set_a_vertices:set_b_vertices" (comma-separated, sorted)
// Correct: "0,2,5,7,8,11:1,3,4,6,9,10"
static int verify_answer(const char* input) {
    // Parse input format: "a,b,c:d,e,f"
    int set_a[12] = {0}, set_b[12] = {0};
    int in_set_a[12] = {0}, in_set_b[12] = {0};
    
    char* colon = strchr(input, ':');
    if (!colon) return 0;
    
    // Parse set A
    char* copy = strdup(input);
    copy[colon - input] = '\0';
    char* tok = strtok(copy, ",");
    int count_a = 0;
    while (tok && count_a < 12) {
        int v = atoi(tok);
        if (v < 0 || v >= 12) { free(copy); return 0; }
        set_a[count_a++] = v;
        in_set_a[v] = 1;
        tok = strtok(NULL, ",");
    }
    
    // Parse set B
    char* part_b = strdup(colon + 1);
    tok = strtok(part_b, ",");
    int count_b = 0;
    while (tok && count_b < 12) {
        int v = atoi(tok);
        if (v < 0 || v >= 12) { free(copy); free(part_b); return 0; }
        set_b[count_b++] = v;
        in_set_b[v] = 1;
        tok = strtok(NULL, ",");
    }
    free(copy);
    free(part_b);
    
    // Check partition is complete (all 12 vertices used exactly once)
    if (count_a + count_b != 12) return 0;
    for (int i = 0; i < 12; i++) {
        if (in_set_a[i] + in_set_b[i] != 1) return 0;
    }
    
    // Check bipartite property: no edges within same set
    for (int i = 0; i < 12; i++) {
        for (int j = i + 1; j < 12; j++) {
            if (adj_matrix[i][j]) {
                // There's an edge i-j, must be in different sets
                if ((in_set_a[i] && in_set_a[j]) || (in_set_b[i] && in_set_b[j])) {
                    return 0;  // Invalid partition!
                }
            }
        }
    }
    
    return 1;  // Valid bipartite partition!
}

// Stored correct answer for key derivation
static char correct_answer[64] = {0};

// ============== FLAG PRINTING ==============
static void print_flag(void) {
    // Flag: L3m0nCTF{v3rt3x_SEED_A_7f2a9b}
    // XOR key = magic_bytes XOR answer_bytes
    // Answer "0,2,5,7,8,11:1,3,4,6,9,10" XOR = 0x3A
    // Final key = 0x40 ^ 0x3A = 0x7A
    unsigned char encoded_flag[] = {
        0x36, 0x49, 0x17, 0x4a, 0x14, 0x39, 0x2e, 0x3c,
        0x01, 0x0c, 0x49, 0x08, 0x0e, 0x49, 0x02, 0x25,
        0x29, 0x3f, 0x3f, 0x3e, 0x25, 0x3b, 0x25, 0x4d,
        0x1c, 0x48, 0x1b, 0x43, 0x18, 0x07, 0x00
    };
    
    uint8_t key = derive_flag_key(correct_answer);
    
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
    printf("           VERTEX - CHALLENGE COMPLETE              \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" You have proven your understanding of the graph.   \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" FLAG: ");
    for (size_t i = 0; encoded_flag[i]; i++) {
        printf("%c", encoded_flag[i] ^ key);
    }
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
}

// ============== MINIMAL UI ==============
static void display_challenge(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║                    VERTEX                          ║\n");
    printf("║            Chromatic Trilogy - Part 1              ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║ Analyze this binary to find the hidden graph.      ║\n");
    printf("║ Prove you understand its structure.                ║\n");
    printf("║                                                    ║\n");
    printf("║ The answer format is hidden in the binary.         ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============== HIDDEN DEBUG (for RE discovery) ==============
__attribute__((used))
void debug_dump_graph(void) {
    init_graph();
    printf("DEBUG: Decrypted adjacency matrix:\n");
    for (int i = 0; i < NUM_VERTICES; i++) {
        for (int j = 0; j < NUM_VERTICES; j++) {
            printf("%d ", adj_matrix[i][j]);
        }
        printf("\n");
    }
}

// ============== MAIN ==============
int main(int argc, char** argv) {
    // Initialize graph (decrypt)
    init_graph();
    
    // Display minimal challenge info
    display_challenge();
    
    // Check for argument
    if (argc >= 2) {
        if (verify_answer(argv[1])) {
            strncpy(correct_answer, argv[1], 63);
            print_flag();
            return 0;
        } else {
            printf("Incorrect.\n");
            return 1;
        }
    }
    
    // Interactive mode
    printf("Enter solution: ");
    fflush(stdout);
    
    char input[64];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        if (strlen(input) > 0) {
            if (verify_answer(input)) {
                strncpy(correct_answer, input, 63);
                print_flag();
                return 0;
            } else {
                printf("Incorrect.\n");
            }
        }
        
        printf("Enter solution: ");
        fflush(stdout);
    }
    
    return 0;
}
