# ✓ Flag Update Complete

## Changes Made

### 1. New Flag Format
- **Old**: `flag{ph4nt0m_r3s0lv3r_1func_m4st3ry}`
- **New**: `L3m0nCTF{ph4nt0m_r3s0lv3r_1func_m4st3ry}`

### 2. XOR Encoding Added
The flag is now **XOR-encoded with key 0x42** to prevent trivial discovery:

```c
// Before: Plain text (visible in strings)
printf("flag{ph4nt0m_r3s0lv3r_1func_m4st3ry}\n");

// After: XOR-encoded (hidden from strings)
unsigned char encoded_flag[] = {
    0x0e, 0x71, 0x2f, 0x72, 0x2c, 0x01, 0x16, 0x04, 0x39, 0x32, 0x2a,
    0x76, 0x2c, 0x36, 0x72, 0x2f, 0x1d, 0x30, 0x71, 0x31, 0x72, 0x2e,
    0x34, 0x71, 0x30, 0x1d, 0x73, 0x24, 0x37, 0x2c, 0x21, 0x1d, 0x2f,
    0x76, 0x31, 0x36, 0x71, 0x30, 0x3b, 0x3f, 0x00
};
for (int i = 0; encoded_flag[i]; i++) {
    printf("%c", encoded_flag[i] ^ 0x42);
}
```

### 3. Verification
✅ `strings libmonitor.so | grep -i l3mon` → **No results** (flag hidden!)  
✅ Decoding works correctly  
✅ Fake backdoor also XOR-encoded for consistency

## For CTFd Upload

**Flag to enter in CTFd:**
```
L3m0nCTF{ph4nt0m_r3s0lv3r_1func_m4st3ry}
```

**Distribution Package:**
```bash
cd dist/
zip -r phantom_resolver_challenge.zip libmonitor.so server_daemon.template README.md
```

Upload `phantom_resolver_challenge.zip` to CTFd.
