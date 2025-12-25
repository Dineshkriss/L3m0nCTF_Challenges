# Vault Challenge - Challenger Walkthrough

## Initial Analysis

### Step 1: Basic Reconnaissance
```bash
file vault_v2
# ELF 64-bit LSB pie executable, x86-64, stripped

./vault_v2
# Enter password: test
# Access Denied.

strings vault_v2 | head -20
# No obvious flags or passwords visible
```

### Step 2: Static Analysis with Ghidra/IDA
1. Open the binary in Ghidra
2. Find `main()` function
3. Look for string references like "Enter password" and "Access Denied"
4. Trace the program flow

You'll find:
- User input is read
- Some comparison functions are called
- It always prints "Access Denied"

### Step 3: Identify Two Compare Functions
In the decompiled code, you'll see two separate comparison functions:

**Function A** (`do_secure_check`):
```c
// Constant-time comparison - XORs all chars
int diff = 0;
for (i = 0; i < len; i++) {
    diff |= (input[i] ^ flag[i]);
}
return (diff == 0);
```
This is a **RED HERRING** - it's a secure constant-time compare.

**Function B** (`do_leaky_compare`):
```c
for (i = 0; i < flag_len; i++) {
    if (input[i] == flag[i]) {
        nanosleep(...);  // ← DELAY HERE!
    } else {
        break;
    }
}
```
This is **THE VULNERABILITY** - it delays on correct characters!

### Step 4: Identify the Vulnerability
The `nanosleep()` call creates a timing side-channel:
- Correct character → ~1.5 second delay
- Wrong character → immediate return

### Step 5: Write a Timing Attack Script
```python
#!/usr/bin/env python3
import subprocess
import time
import string

def measure_time(guess):
    start = time.time()
    proc = subprocess.run(
        ['./vault_v2'],
        input=guess.encode() + b'\n',
        capture_output=True,
        timeout=120
    )
    return time.time() - start

charset = string.ascii_letters + string.digits + "{}_"
flag = ""
DELAY = 1.2  # Threshold for correct character

print("[*] Starting timing attack...")

while True:
    best_char = None
    best_time = 0
    
    for c in charset:
        guess = flag + c
        elapsed = measure_time(guess)
        
        if elapsed > best_time:
            best_time = elapsed
            best_char = c
    
    expected = (len(flag) + 1) * 1.5
    if best_time > expected - 0.5:
        flag += best_char
        print(f"[+] Found: {flag}")
        if best_char == '}':
            break
    else:
        print("[-] Attack failed")
        break

print(f"\n[*] FLAG: {flag}")
```

### Step 6: Run the Attack
```bash
python3 timing_attack.py
```

Expected output:
```
[*] Starting timing attack...
[+] Found: L
[+] Found: L3
[+] Found: L3m
...
[*] FLAG: L3m0nCTF{t1m1ng_muk1yum_kumar3}
```

## Key Insights
1. The "secure" compare function is a decoy
2. Look for functions that call `sleep()` or `nanosleep()`
3. Timing differences = exploitable side channel
