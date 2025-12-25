# Zenbleed Challenge Documentation

## Challenge Description

**Name**: Lockdown (vault_v2)  
**Category**: Reverse Engineering / Side-Channel  
**Difficulty**: Easy | 
**Flag**: `CTF{t1m1ng_1s_3v3ryth1ng}`

### Briefing
> "Our vault uses state-of-the-art constant-time comparison to prevent timing attacks. At least, that's what the junior dev said before he went on vacation. Can you verify his implementation is actually secure?"

---

## How It Works

### The Vulnerability: Timing Side-Channel Attack
The challenge contains a **timing leak** that allows character-by-character password brute-forcing by measuring execution time.

**The Flaw**: When a character matches, the program executes a **3-second delay**. By measuring response time, an attacker can determine which characters are correct.

### Execution Flow

```
User Input: "CTF{" (4 chars correct)
                ↓
    ┌─────────────────────────────────┐
    │ Compare char[0]: 'C' == 'C' ✓  │ → 3 second delay
    │ Compare char[1]: 'T' == 'T' ✓  │ → 3 second delay
    │ Compare char[2]: 'F' == 'F' ✓  │ → 3 second delay
    │ Compare char[3]: '{' == '{' ✓  │ → 3 second delay
    └─────────────────────────────────┘
                ↓
         Total: ~12 seconds
```

### The Obfuscation Layers

| Layer | Technique | Purpose |
|-------|-----------|---------|
| **1** | XOR-encrypted flag | Defeats `strings` command |
| **2** | Red herring `do_secure_check()` | Constant-time comparison (decoy) |
| **3** | Hidden `do_leaky_compare()` | The actual vulnerable function |

---

## Implementation Details

### Files Created/Modified

| File | Purpose |
|------|---------|
| `Zenbleed/vault_v2.c` | Main challenge source code |
| `vault_v2_5` | Compiled challenge binary |

### Key Code Components

#### XOR-Encrypted Flag
```c
// "CTF{t1m1ng_1s_3v3ryth1ng}" XOR'd with 0x5A
char encrypted_flag[] = {0x19, 0x0E, 0x1C, 0x21, 0x2E, 0x6B, ...};

// Decryption at runtime
void get_flag_string(char *buffer) {
    strcpy(buffer, encrypted_flag);
    decrypt_vault(buffer);  // XOR with 0x5A
}
```

#### The Vulnerable Function (Hidden)
```c
void do_leaky_compare(const char *input, const char *flag) {
    for (int i = 0; i < strlen(flag); i++) {
        if (i < strlen(input)) {
            if (input[i] == flag[i]) {
                // VULNERABILITY: Delay on correct character!
                do_complex_cache_miss_calculation();  // 3 second sleep
            } else {
                break;  // Wrong character, stop immediately
            }
        } else {
            break;
        }
    }
}
```

#### The Red Herring (Decoy)
```c
int do_secure_check(const char *input, const char *flag) {
    // This LOOKS like a secure constant-time compare
    // Analysts will waste time here thinking this is the vuln
    int diff = 0;
    for (int i = 0; i < strlen(flag); i++) {
        diff |= (input[i] ^ flag[i]);
    }
    return (diff == 0);
}
```

### Build Command
```bashby removing hints and author
gcc -o vault_v2_5 vault_v2.c
```

---

## Deployment Notes

### Production Configuration
1. Adjust delay time if needed (currently 3 seconds per char)
2. Flag length: 25 characters → ~75 seconds for full brute-force
3. No special permissions required

### Docker Setup 
```dockerfile
FROM ubuntu:22.04
COPY vault_v2_5 /challenge/vault_v2_5
RUN chmod 755 /challenge/vault_v2_5
EXPOSE 1337
CMD socat TCP-LISTEN:1337,reuseaddr,fork EXEC:/challenge/vault_v2_5
```

---

## Summary

| Aspect | Details |
|--------|---------|
| **Vulnerability** | Timing Side-Channel |
| **Delay per Char** | 3 seconds |
| **Flag Length** | 25 characters |
| **Obfuscation** | XOR encryption + Red herring |
| **Exploit Method** | Measure execution time per guess |
| **Flag** | `CTF{t1m1ng_1s_3v3ryth1ng}` |
