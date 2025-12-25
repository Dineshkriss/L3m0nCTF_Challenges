#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// External function from libmonitor.so
extern void system_check();

// Placeholder magic bytes at offset 0x1000
// This will be replaced in the template with actual "CHROMA" bytes
__attribute__((section(".magic")))
__attribute__((aligned(4096)))
const char magic_placeholder[16] = "PLACEHOLDER_DATA";

void print_banner() {
    printf("╔════════════════════════════════════════╗\n");
    printf("║   Secure Server Daemon v2.1.3          ║\n");
    printf("║   Monitoring Subsystem Active          ║\n");
    printf("╚════════════════════════════════════════╝\n");
}

void initialize_subsystems() {
    printf("[*] Initializing cryptographic modules...\n");
    usleep(100000);
    printf("[*] Loading security policies...\n");
    usleep(100000);
    printf("[*] Establishing secure channels...\n");
    usleep(100000);
}

int main(int argc, char *argv[]) {
    print_banner();
    
    printf("\n[*] Starting daemon in ");
    
    // Check for --daemon flag
    int daemon_mode = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--daemon") == 0) {
            daemon_mode = 1;
            break;
        }
    }
    
    if (daemon_mode) {
        printf("DAEMON mode\n");
    } else {
        printf("INTERACTIVE mode\n");
        printf("[!] Warning: daemon mode not enabled\n");
        printf("[!] Use --daemon flag for production deployment\n");
    }
    
    printf("\n");
    initialize_subsystems();
    
    printf("\n[*] Running system integrity check...\n");
    
    // This triggers the IFUNC resolver
    system_check();
    
    printf("\n[*] Daemon initialization complete\n");
    
    return 0;
}
