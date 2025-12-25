/*
 * vault-v2 (Timing Attack Challenge)
 * Flag: L3m0nCTF{t1m1ng_muk1yum_kumar3}
 * Vulnerability: 1.5 second delay per correct character
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/* Prevent compiler optimization */
volatile int g_dummy = 0;

/* Timing delay - 1.5 seconds per correct character */
__attribute__((noinline))
void do_delay()
{
    struct timespec ts = {1, 500000000L};
    nanosleep(&ts, NULL);
}

/* THE VULNERABILITY - timing leak on correct characters */
__attribute__((noinline))
void do_leaky_compare(const char *input, const char *flag, int flag_len)
{
    int input_len = strlen(input);
    for (int i = 0; i < flag_len; i++)
    {
        if (i < input_len)
        {
            if (input[i] == flag[i])
            {
                do_delay();
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

/* RED HERRING - constant time compare (decoy) */
__attribute__((noinline))
int do_secure_check(const char *input, const char *flag, int flag_len)
{
    if (strlen(input) != flag_len)
        return 0;
    int diff = 0;
    for (int i = 0; i < flag_len; i++)
        diff |= (input[i] ^ flag[i]);
    return (diff == 0);
}

/* Build flag at runtime - bytes constructed individually to prevent optimization */
__attribute__((noinline))
int get_flag_string(char *buffer)
{
    /* 
     * XOR key split across operations to obfuscate
     * Key = 0x5A (built as 0x2D + 0x2D)
     */
    volatile unsigned char k1 = 0x2D;
    volatile unsigned char k2 = 0x2D;
    unsigned char key = k1 + k2;
    
    volatile unsigned char e[32];
    
    /* Build encrypted bytes one at a time with arithmetic obfuscation */
    /* This prevents the compiler from optimizing into a string constant */
    e[0]  = (unsigned char)(0x10 + 6);     /* 0x16 */
    e[1]  = (unsigned char)(0x60 + 9);     /* 0x69 */
    e[2]  = (unsigned char)(0x30 + 7);     /* 0x37 */
    e[3]  = (unsigned char)(0x60 + 10);    /* 0x6A */
    e[4]  = (unsigned char)(0x30 + 4);     /* 0x34 */
    e[5]  = (unsigned char)(0x10 + 9);     /* 0x19 */
    e[6]  = (unsigned char)(0x00 + 14);    /* 0x0E */
    e[7]  = (unsigned char)(0x10 + 12);    /* 0x1C */
    e[8]  = (unsigned char)(0x20 + 1);     /* 0x21 */
    e[9]  = (unsigned char)(0x20 + 14);    /* 0x2E */
    e[10] = (unsigned char)(0x60 + 11);    /* 0x6B */
    e[11] = (unsigned char)(0x30 + 7);     /* 0x37 */
    e[12] = (unsigned char)(0x60 + 11);    /* 0x6B */
    e[13] = (unsigned char)(0x30 + 4);     /* 0x34 */
    e[14] = (unsigned char)(0x30 + 13);    /* 0x3D */
    e[15] = (unsigned char)(0x00 + 5);     /* 0x05 */
    e[16] = (unsigned char)(0x30 + 7);     /* 0x37 */
    e[17] = (unsigned char)(0x20 + 15);    /* 0x2F */
    e[18] = (unsigned char)(0x30 + 1);     /* 0x31 */
    e[19] = (unsigned char)(0x60 + 11);    /* 0x6B */
    e[20] = (unsigned char)(0x20 + 3);     /* 0x23 */
    e[21] = (unsigned char)(0x20 + 15);    /* 0x2F */
    e[22] = (unsigned char)(0x30 + 7);     /* 0x37 */
    e[23] = (unsigned char)(0x00 + 5);     /* 0x05 */
    e[24] = (unsigned char)(0x30 + 1);     /* 0x31 */
    e[25] = (unsigned char)(0x20 + 15);    /* 0x2F */
    e[26] = (unsigned char)(0x30 + 7);     /* 0x37 */
    e[27] = (unsigned char)(0x30 + 11);    /* 0x3B */
    e[28] = (unsigned char)(0x20 + 8);     /* 0x28 */
    e[29] = (unsigned char)(0x60 + 9);     /* 0x69 */
    e[30] = (unsigned char)(0x20 + 7);     /* 0x27 */
    
    int len = 31;
    
    /* Decrypt and copy to buffer */
    for (int i = 0; i < len; i++)
    {
        buffer[i] = e[i] ^ key;
        g_dummy += buffer[i];  /* Prevent dead code elimination */
    }
    buffer[len] = '\0';
    
    return len;
}

int main(int argc, char **argv)
{
    char user_input[128] = {0};
    printf("Enter password: ");
    fflush(stdout);
    
    int bytes_read = read(0, user_input, 127);
    if (bytes_read > 0)
        user_input[bytes_read - 1] = '\0';

    char flag[128];
    int flag_len = get_flag_string(flag);

    do_leaky_compare(user_input, flag, flag_len);
    do_secure_check(user_input, flag, flag_len);

    printf("\nAccess Denied.\n");
    return 0;
}
