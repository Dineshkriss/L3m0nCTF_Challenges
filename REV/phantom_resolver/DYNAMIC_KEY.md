# ✅ Dynamic Key Generation Implemented

## What Was Added

### 1. Dynamic Key Derivation Function
```c
static uint8_t derive_xor_key() {
    // Reads CHROMA magic bytes from offset 0x3000
    // XORs all 6 bytes together: 'C'^'H'^'R'^'O'^'M'^'A'
    // Result: 0x1A (26 decimal)
    return key;
}
```

### 2. Flag Re-encoded with Dynamic Key
- **Old**: XOR with static key `0x42`
- **New**: XOR with dynamic key `0x1A` (derived from magic bytes)

### 3. Why This is Harder

**Before (simple XOR 0x42):**
```
AI prompt: "Decode bytes X with XOR 0x42"
→ Instant flag
```

**After (dynamic key):**
```
AI must understand:
1. Key is derived from binary content
2. Magic bytes at offset 0x3000
3. All 6 bytes XOR'd together
4. THEN decode the flag

Still solvable, but requires analysis!
```

## Verification

✅ Key calculation: `'C'^'H'^'R'^'O'^'M'^'A' = 0x1A`  
✅ Flag decodes correctly with key `0x1A`  
✅ No plaintext flag in `strings` output  
✅ Binary rebuilt successfully

## Red Herring Bonus

The **fake backdoor** still uses simple XOR `0x42`, creating confusion:
- Real backdoor: Dynamic key from CHROMA
- Fake backdoor: Static XOR 0x42
- Contestants might decode the fake flag first and get misled!

## For Challenges

This makes the challenge **"Hard to Insane"** difficulty because:
- Simple string search: ❌ Fails
- Simple XOR decode: ❌ Gets wrong flag (if they try 0x42)
- Full binary analysis + understanding: ✅ Required for correct solution
