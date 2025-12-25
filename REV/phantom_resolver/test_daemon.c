#include <stdio.h>

extern void system_check_debug();

int main(int argc, char *argv[]) {
    printf("=== TEST DAEMON DEBUG BINARY ===\\n");
    system_check_debug();
    return 0;
}
