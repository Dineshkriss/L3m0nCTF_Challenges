# Phantom Resolver - Beginner-Friendly Solution Walkthrough

**Category:** Reverse Engineering  
**Difficulty:** Hard (500 pts)  
**Time:** 2-4 hours

---

## What You're Given

```
dist/
├── libmonitor.so           # A "monitoring" shared library (stripped)
├── server_daemon.template  # A server binary template
└── README.md               # Challenge description
```

---

## Part 1: Understanding the Challenge

### What is IFUNC?

**IFUNC** (Indirect Function) is a GNU extension that allows a library to choose which function implementation to use **at load time** - before `main()` even runs.

This is exactly what the real **XZ Utils backdoor (CVE-2024-3094)** used!

```
Normal function call:    program calls foo() → foo() runs
IFUNC function call:     program calls foo() → resolver picks foo_v1 or foo_v2 → chosen one runs
```

### The Goal

Make the library's IFUNC resolver choose the **backdoor function** instead of the **normal function**.

---

## Part 2: Initial Exploration

### Step 1: Check File Types

```bash
cd dist/
file *
```

**Output:**
```
libmonitor.so: ELF 64-bit LSB shared object, x86-64, stripped
server_daemon.template: ELF 64-bit LSB executable, x86-64
```

The library is **stripped** - no debug symbols. We'll need to reverse engineer it.

### Step 2: Look for Strings

```bash
strings libmonitor.so | head -50
```

You'll see some interesting strings:
- `/proc/self/exe`
- `/proc/self/cmdline`  
- `--daemon`
- Some fake-looking security messages

**No flag visible!** (It's XOR-encoded)

### Step 3: Check for IFUNC

```bash
readelf -s libmonitor.so 2>/dev/null | grep -i ifunc
# OR
objdump -t libmonitor.so 2>/dev/null | grep -i ifunc
```

If you see an IFUNC symbol, you've confirmed the attack vector.

---

## Part 3: Reverse Engineering with Ghidra

### Step 1: Open Ghidra

```bash
ghidra &
# OR download from: https://ghidra-sre.org/
```

1. Create new project
2. Import `libmonitor.so`
3. Double-click to open in CodeBrowser
4. Click "Yes" to analyze

### Step 2: Find the Resolver Function

**Method A: Search for /proc/self**
- Press `S` (Search → For Strings)
- Search for `/proc/self/exe`
- Double-click result → Find XREF (references)

**Method B: Look at function list**
- Window → Functions
- Look for functions that return function pointers
- IFUNC resolvers typically have signature: `void* (*func)(void)`

### Step 3: Analyze the Resolver

You'll find a function (may be named something like `FUN_00001xxx`) that looks like:

```c
void* resolve_function(void) {
    // Check 1: Can we read our own binary?
    if (!validate_caller()) {
        return normal_function;
    }
    
    // Check 2: Does binary have magic bytes?
    if (!verify_magic_bytes()) {
        return normal_function;
    }
    
    // Check 3: Is --daemon in command line?
    if (!check_cmdline()) {
        return normal_function;
    }
    
    // All checks passed!
    return backdoor_function;
}
```

### Step 4: Understand Each Check

#### Check 1: Binary Exists
```c
int fd = open("/proc/self/exe", O_RDONLY);
// Just checks that the binary is readable - always passes
```
✅ **Always passes** - no action needed

#### Check 2: Magic Bytes (THE KEY!)

Look for code like:
```c
lseek(fd, 0x3000, SEEK_SET);    // ← NOTE THIS OFFSET!
read(fd, buffer, 6);             // ← Reads 6 bytes

// XOR-encoded comparison
uint8_t expected[] = {0x01, 0x0a, 0x10, 0x0d, 0x0f, 0x03};
for (int i = 0; i < 6; i++) expected[i] ^= 0x42;
// Decodes to: "CHROMA"
```

**Key discovery:** Binary must have `"CHROMA"` at offset `0x3000`!

#### Check 3: Command Line
```c
read("/proc/self/cmdline", ...);
strstr(cmdline, "--daemon");     // ← Needs this flag!
```

**Key discovery:** Must run with `--daemon` argument!

---

## Part 4: Solving the Challenge

### Step 1: Create Patch Script

Create `solve.py`:

```python
#!/usr/bin/env python3
"""
Phantom Resolver Solver
Patches the template binary and runs it to get the flag.
"""

import os
import subprocess

# Constants discovered from reverse engineering
MAGIC_OFFSET = 0x3000      # Offset where magic bytes must be
MAGIC_BYTES = b"CHROMA"    # The magic string (XOR-decoded from library)

def main():
    print("[*] Phantom Resolver - Automated Solver")
    print("=" * 50)
    
    # Step 1: Read template
    if not os.path.exists('server_daemon.template'):
        print("[!] Error: server_daemon.template not found!")
        print("[!] Run this script from the dist/ directory")
        return
    
    with open('server_daemon.template', 'rb') as f:
        data = bytearray(f.read())
    
    print(f"[*] Loaded template ({len(data)} bytes)")
    
    # Step 2: Patch magic bytes at offset 0x3000
    print(f"[*] Patching '{MAGIC_BYTES.decode()}' at offset 0x{MAGIC_OFFSET:X}")
    data[MAGIC_OFFSET:MAGIC_OFFSET + len(MAGIC_BYTES)] = MAGIC_BYTES
    
    # Step 3: Write patched binary
    with open('server_daemon', 'wb') as f:
        f.write(data)
    os.chmod('server_daemon', 0o755)
    print("[+] Created patched binary: server_daemon")
    
    # Step 4: Run with the required --daemon flag
    print("[*] Running: ./server_daemon --daemon")
    print("=" * 50)
    
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = '.'  # Ensure library is found
    
    result = subprocess.run(
        ['./server_daemon', '--daemon'],
        env=env,
        capture_output=True,
        text=True
    )
    
    print(result.stdout)
    
    if 'flag{' in result.stdout or 'CTF{' in result.stdout or 'L3m0n' in result.stdout:
        print("=" * 50)
        print("[+] SUCCESS! Flag captured above! 🎉")
    else:
        print("[!] No flag found - check for errors")

if __name__ == '__main__':
    main()
```

### Step 2: Run the Solver

```bash
cd dist/
python3 solve.py
```

**Expected Output:**
```
[*] Phantom Resolver - Automated Solver
==================================================
[*] Loaded template (20000 bytes)
[*] Patching 'CHROMA' at offset 0x3000
[+] Created patched binary: server_daemon
[*] Running: ./server_daemon --daemon
==================================================
L3m0nCTF{ph4nt0m_r3s0lv3r_1func_m4st3ry}
==================================================
[+] SUCCESS! Flag captured above! 🎉
```

---

## Part 5: Manual Solution (No Script)

If you prefer doing it manually:

### Option A: Using Python

```python
python3 -c "
data = bytearray(open('server_daemon.template','rb').read())
data[0x3000:0x3006] = b'CHROMA'
open('server_daemon','wb').write(data)
import os; os.chmod('server_daemon', 0o755)
print('Done!')
"
```

### Option B: Using dd and printf

```bash
cp server_daemon.template server_daemon
printf 'CHROMA' | dd of=server_daemon bs=1 seek=$((0x3000)) conv=notrunc
chmod +x server_daemon
```

### Option C: Using hexedit

```bash
hexedit server_daemon.template
# Press Ctrl+G, type 3000, press Enter
# Type: CHROMA
# Press Ctrl+X to save, Y to confirm
mv server_daemon.template server_daemon
chmod +x server_daemon
```

Then run:
```bash
LD_LIBRARY_PATH=. ./server_daemon --daemon
```

---

## Part 6: Understanding Why This Works

```
┌─────────────────────────────────────────────────────────────┐
│                    EXECUTION FLOW                           │
├─────────────────────────────────────────────────────────────┤
│  1. You run: ./server_daemon --daemon                       │
│                                                             │
│  2. Linux loader sees libmonitor.so dependency              │
│                                                             │
│  3. Loader sees IFUNC on system_check()                     │
│                                                             │
│  4. Loader calls resolve_function() BEFORE main()           │
│                                                             │
│  5. resolve_function() runs three checks:                   │
│     ✓ Binary readable? YES                                  │
│     ✓ "CHROMA" at 0x3000? YES (we patched it!)             │
│     ✓ "--daemon" in cmdline? YES (we passed it!)           │
│                                                             │
│  6. Returns backdoor_function instead of normal_function    │
│                                                             │
│  7. When server calls system_check(), backdoor runs         │
│                                                             │
│  8. Backdoor XOR-decodes and prints the flag!               │
└─────────────────────────────────────────────────────────────┘
```

---

## Common Mistakes to Avoid

| Mistake | Fix |
|---------|-----|
| "No such file or directory" | Run from the `dist/` directory |
| "libmonitor.so not found" | Set `LD_LIBRARY_PATH=.` |
| Binary runs but no flag | Check you patched at **0x3000** (not 0x1000) |
| Still no flag | Make sure you included `--daemon` flag |
| Got a fake flag | You found a red herring function! Re-analyze |

---

## Red Herrings in the Binary

The library contains **intentional distractions**:

1. **`fake_backdoor_function`** - Prints a wrong flag
2. **Fake AES constants** - S-box arrays that lead nowhere  
3. **`fake_decrypt_payload`** - Looks like crypto, does nothing useful
4. **Environment variable checks** - Checks LD_PRELOAD but ignores result
5. **Anti-debug code** - Looks scary but always passes

**Pro tip:** Focus on what the IFUNC resolver **actually uses**, not just what exists in the binary.

---

## Skills You Practiced

- ✅ ELF binary analysis
- ✅ Reverse engineering with Ghidra
- ✅ Understanding IFUNC mechanism
- ✅ XOR deobfuscation
- ✅ Binary patching
- ✅ Python scripting

---

## Flag

```
L3m0nCTF{ph4nt0m_r3s0lv3r_1func_m4st3ry}
```

---

## Further Reading

- [XZ Utils Backdoor (CVE-2024-3094)](https://en.wikipedia.org/wiki/XZ_Utils_backdoor)
- [GNU IFUNC Documentation](https://sourceware.org/glibc/wiki/GNU_IFUNC)
- [Ghidra Tutorial](https://ghidra-sre.org/CheatSheet.html)
