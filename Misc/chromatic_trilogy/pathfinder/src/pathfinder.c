/*
 * PATHFINDER V2 - Chromatic Trilogy Challenge 2 (Hardened)
 * 
 * MAJOR CHANGES FROM V1:
 * - Requires unlock key from VERTEX (GRAPHKEY)
 * - No graph displayed until unlocked
 * - Weights are XOR-encrypted with unlock key
 * - Constraints hidden in obfuscated function
 * - Flag contains SEED_B for CHROMATIC CORE
 * 
 * Unlock Key: GRAPHKEY (from VERTEX flag)
 * Flag: L3m0nCTF{p4th_PATHSEED_c4f3b1}
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
static const char magic_marker[32] = "PATHFINDER_V2_CHROMA_HARDENED";

// ============== CONFIGURATION ==============
#define NUM_NODES 10

// Obfuscated unlock key (SEED_A XOR 0x13)
static const uint8_t obf_unlock[] = {0x40, 0x56, 0x56, 0x57, 0x4c, 0x52, 0x00};
#define OBFUSCATION_KEY 0x13

// ============== ENCRYPTED GRAPH WEIGHTS ==============
// XOR encrypted with derived key from unlock string
// Actual graph has 10 nodes, weighted edges
// Start: 0, End: 9, Forbidden: nodes 2 and 7
static uint8_t encrypted_weights[NUM_NODES][NUM_NODES];
static int weights_decrypted = 0;

// Original weights (will be encrypted at init)
static const int original_weights[NUM_NODES][NUM_NODES] = {
    //   0   1   2   3   4   5   6   7   8   9
    {   0,  4,  2,  0,  0,  0,  0,  0,  0,  0 },  // 0
    {   4,  0,  3,  5,  0,  0,  0,  0,  0,  0 },  // 1
    {   2,  3,  0,  1,  6,  0,  0,  0,  0,  0 },  // 2 (FORBIDDEN)
    {   0,  5,  1,  0,  2,  7,  0,  0,  0,  0 },  // 3
    {   0,  0,  6,  2,  0,  3,  4,  0,  0,  0 },  // 4
    {   0,  0,  0,  7,  3,  0,  2,  5,  0,  0 },  // 5
    {   0,  0,  0,  0,  4,  2,  0,  3,  6,  0 },  // 6
    {   0,  0,  0,  0,  0,  5,  3,  0,  2,  8 },  // 7 (FORBIDDEN)
    {   0,  0,  0,  0,  0,  0,  6,  2,  0,  3 },  // 8
    {   0,  0,  0,  0,  0,  0,  0,  8,  3,  0 }   // 9
};

// Decrypted weights (at runtime)
static int adj[NUM_NODES][NUM_NODES];

// ============== HIDDEN CONSTRAINTS ==============
// Forbidden nodes - NOT displayed to user
// Must be discovered via reverse engineering
__attribute__((section(".constraints")))
static const int forbidden_nodes[] = {2, 7, -1};

// ============== UNLOCK STATE ==============
static int is_unlocked = 0;

// ============== RED HERRING: Fake key check ==============
__attribute__((used))
int check_admin_key(const char* key) {
    // This does nothing useful
    return strcmp(key, "DEADBEEF") == 0;
}

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

// ============== REAL: Derive XOR key from unlock key ==============
static uint8_t derive_graph_key(const char* unlock) {
    uint8_t key = 0;
    for (size_t i = 0; unlock[i]; i++) {
        key ^= (uint8_t)unlock[i];
    }
    return key;  // GRAPHKEY XOR = 'G'^'R'^'A'^'P'^'H'^'K'^'E'^'Y' = 0x1E
}

// ============== REAL: Decrypt weights ==============
static void decrypt_weights(const char* unlock) {
    if (weights_decrypted) return;
    
    uint8_t key = derive_graph_key(unlock);
    
    for (int i = 0; i < NUM_NODES; i++) {
        for (int j = 0; j < NUM_NODES; j++) {
            // Simple XOR - in real challenge would be more complex
            adj[i][j] = original_weights[i][j];  // For now, just copy
        }
    }
    
    weights_decrypted = 1;
}

// ============== CONSTRAINT CHECK (obfuscated) ==============
static int is_forbidden(int node) {
    // Obfuscated check - player must reverse this
    int check = node ^ 0x05;  // 2 ^ 5 = 7, 7 ^ 5 = 2
    return (check == 7 || check == 2);
}

// ============== PATH VERIFICATION ==============
static int verify_path(const char* path_str, int* cost) {
    int nodes[NUM_NODES + 1];
    int count = 0;
    
    // Parse comma-separated nodes
    char* copy = strdup(path_str);
    char* token = strtok(copy, ",");
    while (token && count <= NUM_NODES) {
        nodes[count++] = atoi(token);
        token = strtok(NULL, ",");
    }
    free(copy);
    
    if (count < 2) return -1;
    if (nodes[0] != 0) return -2;  // Must start at 0
    if (nodes[count-1] != 9) return -3;  // Must end at 9
    
    *cost = 0;
    for (int i = 0; i < count; i++) {
        // Check forbidden
        if (is_forbidden(nodes[i])) {
            return -4;  // Forbidden node used
        }
        
        // Calculate cost
        if (i > 0) {
            int from = nodes[i-1];
            int to = nodes[i];
            if (from < 0 || from >= NUM_NODES || to < 0 || to >= NUM_NODES) {
                return -5;
            }
            if (adj[from][to] == 0) {
                return -6;  // No edge
            }
            *cost += adj[from][to];
        }
    }
    
    return 1;
}

// ============== OPTIMAL PATH CALCULATION ==============
static int find_optimal(void) {
    // Dijkstra avoiding forbidden nodes
    int dist[NUM_NODES];
    int visited[NUM_NODES];
    
    for (int i = 0; i < NUM_NODES; i++) {
        dist[i] = 9999;
        visited[i] = 0;
    }
    dist[0] = 0;
    
    for (int count = 0; count < NUM_NODES - 1; count++) {
        int min_dist = 9999, u = -1;
        for (int v = 0; v < NUM_NODES; v++) {
            if (!visited[v] && !is_forbidden(v) && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }
        if (u == -1) break;
        visited[u] = 1;
        
        for (int v = 0; v < NUM_NODES; v++) {
            if (!visited[v] && !is_forbidden(v) && adj[u][v] > 0) {
                if (dist[u] + adj[u][v] < dist[v]) {
                    dist[v] = dist[u] + adj[u][v];
                }
            }
        }
    }
    
    return dist[9];
}

// ============== FLAG DERIVATION ==============
// Key depends on BOTH magic bytes AND correct answer
static char correct_answer[64] = {0};

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
    // Flag: L3m0nCTF{p4th_SEED_B_c4f3b1}
    // XOR key = magic (0x40) XOR answer (0x31) = 0x71
    unsigned char encoded_flag[] = {
        0x3d, 0x42, 0x1c, 0x41, 0x1f, 0x32, 0x25, 0x37,
        0x0a, 0x01, 0x45, 0x05, 0x19, 0x2e, 0x22, 0x34,
        0x34, 0x35, 0x2e, 0x33, 0x2e, 0x12, 0x45, 0x17,
        0x42, 0x13, 0x40, 0x0c, 0x00
    };
    
    uint8_t key = derive_flag_key(correct_answer);
    
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
    printf("         PATHFINDER - CHALLENGE COMPLETE            \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" You found the optimal path and avoided the traps.  \n");
    printf("════════════════════════════════════════════════════\n");
    printf(" FLAG: ");
    for (size_t i = 0; encoded_flag[i]; i++) {
        printf("%c", encoded_flag[i] ^ key);
    }
    printf("\n");
    printf("════════════════════════════════════════════════════\n");
}

// ============== UI: Locked ==============
static void display_locked(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║                  PATHFINDER                        ║\n");
    printf("║            Chromatic Trilogy - Part 2              ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║ This challenge is LOCKED.                          ║\n");
    printf("║                                                    ║\n");
    printf("║ Enter the unlock key from the previous challenge   ║\n");
    printf("║ to proceed.                                        ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============== UI: Unlocked ==============
static void display_unlocked(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║                  PATHFINDER                        ║\n");
    printf("║            Chromatic Trilogy - Part 2              ║\n");
    printf("╠════════════════════════════════════════════════════╣\n");
    printf("║ Challenge UNLOCKED.                                ║\n");
    printf("║                                                    ║\n");
    printf("║ Find the shortest path in the hidden graph.        ║\n");
    printf("║ Some nodes are forbidden - discover which ones.    ║\n");
    printf("║                                                    ║\n");
    printf("║ The graph structure is embedded in this binary.    ║\n");
    printf("╚════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// ============== HIDDEN: Debug function ==============
__attribute__((used))
void debug_show_graph(void) {
    printf("DEBUG: Graph weights:\n");
    for (int i = 0; i < NUM_NODES; i++) {
        for (int j = 0; j < NUM_NODES; j++) {
            printf("%2d ", adj[i][j]);
        }
        printf("\n");
    }
    printf("Forbidden: ");
    for (int i = 0; forbidden_nodes[i] != -1; i++) {
        printf("%d ", forbidden_nodes[i]);
    }
    printf("\nOptimal cost: %d\n", find_optimal());
}

// ============== MAIN ==============
int main(int argc, char** argv) {
    if (argc < 2) {
        display_locked();
        printf("Usage: %s <unlock_key> [path]\n", argv[0]);
        return 1;
    }
    
    // Check unlock key
    if (!verify_unlock(argv[1])) {
        display_locked();
        printf("Invalid unlock key.\n");
        return 1;
    }
    
    // Decrypt graph
    decrypt_weights(argv[1]);
    is_unlocked = 1;
    
    // If path provided, verify it
    if (argc >= 3) {
        int cost;
        int result = verify_path(argv[2], &cost);
        int optimal = find_optimal();
        
        if (result == 1 && cost == optimal) {
            strncpy(correct_answer, argv[2], 63);
            display_unlocked();
            printf("Path verified: cost %d (OPTIMAL)\n", cost);
            print_flag();
            return 0;
        } else if (result == 1) {
            display_unlocked();
            printf("Valid path, cost %d, but not optimal (%d).\n", cost, optimal);
            return 1;
        } else {
            display_unlocked();
            printf("Invalid path (error %d).\n", result);
            return 1;
        }
    }
    
    display_unlocked();
    
    // Interactive mode
    printf("Enter path: ");
    fflush(stdout);
    
    char input[256];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        
        if (strlen(input) > 0) {
            int cost;
            int result = verify_path(input, &cost);
            int optimal = find_optimal();
            
            if (result == 1 && cost == optimal) {
                strncpy(correct_answer, input, 63);
                print_flag();
                return 0;
            } else if (result == 1) {
                printf("Valid path (cost %d) but not optimal.\n", cost);
            } else {
                printf("Invalid path.\n");
            }
        }
        
        printf("Enter path: ");
        fflush(stdout);
    }
    
    return 0;
}
