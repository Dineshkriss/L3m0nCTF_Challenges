#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "NativeSecurity.h"

/*
 * Obfuscated Blocklist
 * We use a simple XOR cipher to hide the keywords.
 * Key: 0x42
 */

// "jndi" ^ 0x42
unsigned char k_jndi[] = {0x28, 0x2C, 0x26, 0x2B, 0x00};
// "ldap" ^ 0x42
unsigned char k_ldap[] = {0x2E, 0x26, 0x23, 0x32, 0x00};
// "rmi" ^ 0x42
unsigned char k_rmi[] = {0x30, 0x2F, 0x2B, 0x00};
// "lower" ^ 0x42
unsigned char k_lower[] = {0x2E, 0x2D, 0x35, 0x27, 0x30, 0x00};
// "upper" ^ 0x42
unsigned char k_upper[] = {0x37, 0x32, 0x32, 0x27, 0x30, 0x00};
// "env" ^ 0x42
unsigned char k_env[] = {0x27, 0x2C, 0x34, 0xa00};
// "java" ^ 0x42
unsigned char k_java[] = {0x28, 0x23, 0x34, 0x23, 0x00};
// "ctx" ^ 0x42
unsigned char k_ctx[] = {0x21, 0x36, 0x3A, 0x00};
// "::-" ^ 0x42
unsigned char k_col[] = {0x78, 0x78, 0x6F, 0x00};

void decrypt(unsigned char *str, char *out) {
    int i = 0;
    while (str[i] != 0) {
        out[i] = str[i] ^ 0x42;
        i++;
    }
    out[i] = '\0';
}

JNIEXPORT jboolean JNICALL Java_NativeSecurity_check(JNIEnv *env, jclass cls, jstring input) {
    const char *str = (*env)->GetStringUTFChars(env, input, 0);
    if (!str) return 0;

    // Decrypt keywords
    char d_jndi[10], d_ldap[10], d_rmi[10], d_lower[10], d_upper[10], d_env[10], d_java[10], d_ctx[10], d_col[10];
    
    decrypt(k_jndi, d_jndi);
    decrypt(k_ldap, d_ldap);
    decrypt(k_rmi, d_rmi);
    decrypt(k_lower, d_lower);
    decrypt(k_upper, d_upper);
    decrypt(k_env, d_env);
    decrypt(k_java, d_java);
    decrypt(k_ctx, d_ctx);
    decrypt(k_col, d_col);

    jboolean result = 0;

    // The "Paranoid" Check
    if (strstr(str, d_jndi) || strstr(str, d_ldap) || strstr(str, d_rmi) ||
        strstr(str, d_lower) || strstr(str, d_upper) || strstr(str, d_env) ||
        strstr(str, d_java) || strstr(str, d_ctx) || strstr(str, d_col)) {
        result = 1; // Blocked
    }

    // "k8s" and "date" are MISSING from this list!
    // This is the bypass.

    (*env)->ReleaseStringUTFChars(env, input, str);
    return result;
}
