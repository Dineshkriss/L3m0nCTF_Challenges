# 🔓 VAULT BREACH - CTF Writeup

**Category:** Reverse Engineering / Web  
**Difficulty:** Hard  
**Flag Format:** `L3m0nCTF{...}`

---

## Initial Reconnaissance

Opening the challenge URL, I'm greeted with a cyberpunk-styled terminal interface. There's an input field asking for an "access key" and an EXECUTE button. The hint says the flag format is `L3m0nCTF{...}`.

Let me open DevTools (F12) and see what we're working with.

### Network Tab Analysis
Refreshing the page, I see these files loading:
- `index.html` - The UI
- `styles.css` - Styling
- `script.js` - JavaScript logic
- `vault.wasm` - WebAssembly binary ← **This is interesting!**

### JavaScript Analysis

Looking at `script.js`, I find the key function:

```javascript
async function loadWasm() {
    wasmModule = await WebAssembly.instantiateStreaming(fetch("vault.wasm"), {
        env: {
            x: _0x4f72  // Obfuscated function name
        }
    });
}
```

The WASM imports a function called `x` from JavaScript. Let me find what `_0x4f72` does:

```javascript
const _0x4f72 = (n) => { 
    let r=0; 
    while(n!==0) {
        r^=n&1; 
        n>>>=1;
    } 
    return r; 
};
```

This calculates the **parity** of a number (0 if even number of 1-bits, 1 if odd). This will be important later!

---

## Extracting the WASM

Let me download `vault.wasm` and analyze it:

```bash
curl -O http://localhost:9999/vault.wasm
```

### Disassembling to WAT

Using the WebAssembly Binary Toolkit:

```bash
wasm2wat vault.wasm -o vault.wat
```

Now I have human-readable WebAssembly text format.

---

## Analyzing the WASM Structure

### Memory Layout

From the WAT file, I see data sections:

```wat
(data (i32.const 3584) 
  "\16\0F\12\2E\F3\08\7E\CA...")  ;; Encrypted flag

(data (i32.const 3700)
  "\5A\3C\7F\1E\9D\4B\2A\8C\6E\0D\F3\A7\59\C1\3B\82")  ;; XOR key

(data (i32.const 3720)
  "\4C\33\6D\30\6E\43\54\46\7B")  ;; "L3m0nCTF{" in hex
```

So we have:
- **Offset 3584:** Encrypted flag data
- **Offset 3700:** A 16-byte XOR key
- **Offset 3720:** The expected prefix "L3m0nCTF{"

### Exported Function

The `unlock` function is exported - this is what validates our input.

---

## Understanding Opaque Predicates

Looking at the code, I see patterns like:

```wat
(if (call $parity (i32.const 0x100))
  (then
    ;; Real code path
  )
  (else
    ;; Dead code (never executed)
  )
)
```

The WASM calls the JavaScript `parity` function with constants. Let me compute their values:

```javascript
function parity(n) {
    let r = 0;
    while (n !== 0) { r ^= n & 1; n >>>= 1; }
    return r;
}

console.log("0x100:", parity(0x100));  // 1 (TRUE)
console.log("0x101:", parity(0x101));  // 0 (FALSE)
console.log("0x103:", parity(0x103));  // 1 (TRUE)
console.log("0x165:", parity(0x165));  // 1 (TRUE)
```

Now I know which branches are actually taken!

---

## Tracing the Verification Logic

After removing dead code, the `unlock` function does:

### Stage 1: Length Check
```wat
(if (i32.ne (local.get $len) (i32.const 40))
  (return (i32.const 0)))  ;; Flag must be 40 characters
```

### Stage 2: Prefix Check
Compares first 9 bytes against `L3m0nCTF{`

### Stage 3: Suffix Check
Last character must be `}` (0x7D)

### Stage 4: Content Verification
```wat
;; XOR input with key at offset 3700
(local.set $transformed
  (i32.xor (local.get $input_byte) (local.get $key_byte)))

;; Compare with encrypted flag at offset 3584
(if (i32.ne (local.get $transformed) (local.get $enc_byte))
  (br $fail))
```

### Stage 5: Hash Check
Computes a rolling hash and checks `(hash & 0xFFFF) == 0xD9CE`

---

## Extracting the Flag

Now I have everything I need to decrypt the flag!

### Solver Script

```javascript
// Data from WASM memory
const ENC_FLAG = [
  0x16, 0x0F, 0x12, 0x2E, 0xF3, 0x08, 0x7E, 0xCA,
  0x15, 0x7A, 0xC7, 0xD4, 0x34, 0x9E, 0x0B, 0xF2,
  0x6E, 0x4D, 0x0A, 0x2D, 0xC2, 0x3B, 0x58, 0xBF,
  0x0A, 0x3C, 0x90, 0x93, 0x2D, 0xF2, 0x48, 0xDD,
  0x6E, 0x4E, 0x4C, 0x41, 0xFB, 0x3E, 0x44, 0xF1
];

const XOR_KEY = [
  0x5A, 0x3C, 0x7F, 0x1E, 0x9D, 0x4B, 0x2A, 0x8C,
  0x6E, 0x0D, 0xF3, 0xA7, 0x59, 0xC1, 0x3B, 0x82
];

// Decrypt: XOR encrypted bytes with key
let flag = "";
for (let i = 0; i < ENC_FLAG.length; i++) {
  flag += String.fromCharCode(ENC_FLAG[i] ^ XOR_KEY[i % 16]);
}

console.log("FLAG:", flag);
```

### Running the Solver

```
$ node solve.js
FLAG: L3m0nCTF{w4sm_0p4qu3_pr3d1c4t3s_4r3_fun}
```

---

## Verification

Let me verify by entering the flag in the challenge:

![Success](/home/radkriss/.gemini/antigravity/brain/4b2bdf85-9e13-465c-94c2-f6ce02980218/success_overlay_1766067132266.png)

**ACCESS GRANTED! 🎉**

---

## Flag

```
L3m0nCTF{w4sm_0p4qu3_pr3d1c4t3s_4r3_fun}
```

---

## Key Takeaways

1. **Opaque Predicates** - Branches that look conditional but always go one way. Identify the oracle function and compute its outputs for the constants used.

2. **XOR Encryption** - Classic reversible encryption. If `plaintext ^ key = ciphertext`, then `ciphertext ^ key = plaintext`.

3. **WASM Analysis** - Use `wasm2wat` to convert binary to readable text. WASM is stack-based, so follow the data flow.

4. **Dynamic Analysis** - Could also hook the `x()` function to log all calls and understand execution flow.

---

## Tools Used

| Tool | Purpose |
|------|---------|
| Browser DevTools | Initial recon |
| `wasm2wat` | Disassemble WASM |
| Node.js | Run solver script |
| Patience | Understanding the obfuscation 😅 |
