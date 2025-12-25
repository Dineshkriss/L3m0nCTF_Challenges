# Zenbleed Challenge Documentation

## Challenge Description

**Name**: Lockdown (vault_v2)  
**Category**: Reverse Engineering / Side-Channel  
**Difficulty**: Easy
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
```bash
gcc -o vault_v2_5 vault_v2.c
```

---

## Challenger Walkthrough

### Step 1: Initial Reconnaissance
```bash
file vault_v2_5
# Output: ELF 64-bit LSB executable...

./vault_v2_5
# Enter password: test
# Access Denied.
```

### Step 2: Static Analysis (Ghidra/IDA)
1. Open in Ghidra
2. Find `main()` → see two compare functions called
3. **Red Herring**: `do_secure_check()` looks constant-time
4. **The Key**: Find `do_leaky_compare()` with delay call

### Step 3: Identify the Timing Leak
In Ghidra, trace `do_leaky_compare()`:
```
if (input[i] == flag[i])
    → calls do_complex_cache_miss_calculation()
    → contains nanosleep() with 3 second delay!
```

### Step 4: Write Timing Attack Script

```python
#!/usr/bin/env python3
import subprocess
import time
import string

def measure_time(guess):
    start = time.time()
    proc = subprocess.run(
        ['./vault_v2_5'],
        input=guess.encode() + b'\n',
        capture_output=True,
        timeout=120
    )
    return time.time() - start

charset = string.ascii_letters + string.digits + "{}_"
flag = ""
DELAY_THRESHOLD = 2.5  # Correct char adds ~3 seconds

print("[*] Starting timing attack...")

while True:
    best_char = None
    best_time = 0
    
    for c in charset:
        guess = flag + c
        elapsed = measure_time(guess)
        print(f"  Trying: {guess} → {elapsed:.2f}s")
        
        if elapsed > best_time:
            best_time = elapsed
            best_char = c
    
    # Check if we found a correct character
    expected_time = (len(flag) + 1) * 3  # Each correct = ~3s
    if best_time > expected_time - 1:
        flag += best_char
        print(f"[+] Found: {flag} (time: {best_time:.2f}s)")
        
        if best_char == '}':
            break
    else:
        print("[-] No timing difference detected, attack may have failed")
        break

print(f"\n[*] FLAG: {flag}")
```

### Step 5: Execute Attack

```bash
python3 timing_attack.py
```

**Expected Output**:
```
[*] Starting timing attack...
  Trying: A → 0.01s
  Trying: B → 0.01s
  Trying: C → 3.01s    ← FOUND!
[+] Found: C (time: 3.01s)
  Trying: CA → 3.01s
  Trying: CB → 3.01s
  Trying: CT → 6.01s   ← FOUND!
[+] Found: CT (time: 6.01s)
...
[*] FLAG: CTF{t1m1ng_1s_3v3ryth1ng}
```

---

## Verification

### Manual Test Cases

| Input | Expected Time | Reason |
|-------|---------------|--------|
| `A` | ~0.00s | Wrong first char |
| `C` | ~3.00s | First char correct |
| `CT` | ~6.00s | Two chars correct |
| `CTF{t1m1ng_1s_3v3ryth1ng}` | ~78.00s | All 26 chars correct |

### Quick Verification
```bash
# Wrong character (immediate)
time echo "A" | ./vault_v2_5
# real    0m0.001s

# Correct first character (3 second delay)
time echo "C" | ./vault_v2_5
# real    0m3.001s
```

---

## Deployment Notes

### Production Configuration
1. Adjust delay time if needed (currently 3 seconds per char)
2. Flag length: 25 characters → ~75 seconds for full brute-force
3. No special permissions required

### Docker Setup (Recommended)
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
