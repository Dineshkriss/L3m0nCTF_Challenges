# 🎯 VAULT BREACH - Challenger Walkthrough

A step-by-step guide for solving this WASM reverse engineering challenge.

---

## 🧠 Reverse Engineering Concepts Used

### 1. **Opaque Predicates**
The WASM uses a clever obfuscation technique called **opaque predicates**. Every branch decision calls an imported function `x()` with a constant:

```wat
i32.const 2891    ;; Load constant
call 0            ;; Call x(2891)
if                ;; Branch based on result
  call 1
else
  call 15
end
```

The `x()` function computes the **parity** (number of 1-bits mod 2) of its input:
```javascript
x(number) {
    let result = 0;
    while (number !== 0) {
        result ^= number & 1;
        number >>>= 1;
    }
    return result;  // Returns 0 or 1
}
```

Since inputs are constants, branches are **deterministic** but appear random without understanding `x()`.

---

### 2. **Control Flow Flattening**
The unlock function (`func 23`) doesn't verify the flag directly. Instead, it:
1. Calls multiple sub-functions in sequence
2. Each call's path depends on opaque predicates
3. Some functions transform input, others are decoys

---

### 3. **Encrypted Flag Comparison**
The flag isn't stored in plaintext. At memory offset `3584`, there's encrypted data:
```
\b5\03\00\00\e8\c6f\0c\d7\c1\c7d\9d\11\1c\be...
```
Your input is transformed and compared against this blob.

---

### 4. **Import Dependency**
The WASM imports `x()` from JavaScript. This means:
- You can **hook it** to trace all calls
- You can **modify it** to break the challenge (if you want to cheat!)

---

## 🛠️ Solution Approach

### Step 1: Initial Reconnaissance

Open browser DevTools (F12). Look at the Network tab to confirm `vault.wasm` loads.

**Ask Gemini:**
> "I have a WASM file that takes a flag input and returns true/false. How do I analyze it?"

Gemini will suggest tools like `wasm2wat` or browser debugging.

---

### Step 2: Extract and Disassemble WASM

```bash
# Install wabt (WebAssembly Binary Toolkit)
sudo apt install wabt

# Convert binary to text format
wasm2wat vault.wasm -o vault.wat
```

**Ask Gemini:**
> "Here's my WAT file. What does the `unlock` function do?"
> (paste the output)

---

### Step 3: Understand the `x()` Function

Look at `script.js` - you'll find:
```javascript
x(number) {
    let result = 0;
    while (number !== 0) {
        result ^= number & 1;
        number >>>= 1;
    }
    return result;
}
```

**Ask Gemini:**
> "What does this function compute? `result ^= number & 1; number >>>= 1;`"

Gemini will explain it's **popcount parity** - returns 1 if odd number of bits, 0 if even.

---

### Step 4: Trace Execution with Hooks

Modify `script.js` to trace all `x()` calls:
```javascript
x(number) {
    let result = 0;
    while (number !== 0) {
        result ^= number & 1;
        number >>>= 1;
    }
    console.log(`x(${number}) = ${result}`);  // ADD THIS
    return result;
}
```

Run the challenge with a test input and observe the pattern.

---

### Step 5: Simplify the Control Flow

Since `x(constant)` always returns the same value, you can:

1. Pre-compute all constants: `x(2891)`, `x(2898)`, etc.
2. Replace each `if` with its deterministic branch
3. This reveals the **actual** execution path

**Ask Gemini:**
> "Given this WAT snippet, if x(2891)=1 and x(2898)=0, which functions are called?"

---

### Step 6: Reverse the Transformation

The actual verification logic transforms your input and compares it to encrypted data.

Key insight: The transformations are often **invertible**:
- XOR with key → XOR with same key reverses it
- Add constant → Subtract constant reverses it

**Ask Gemini:**
> "This WASM loads my input, XORs each byte with a value, then compares to encrypted data. How do I reverse this to find the original flag?"

---

### Step 7: Extract the Flag

Two approaches:

**A) Dynamic Analysis (Easier)**
Hook the comparison function and dump what your input is being compared against:
```javascript
// Add to script.js before WASM loads
WebAssembly.instantiateStreaming = async (source, imports) => {
    const result = await WebAssembly.instantiateStreaming(source, imports);
    // Read memory at offset 3584
    const memory = new Uint8Array(result.instance.exports.memory.buffer);
    console.log("Encrypted data:", memory.slice(3584, 3700));
    return result;
};
```

**B) Static Analysis (Harder)**
Fully reverse the transformation logic and apply it to the encrypted blob.

---

## 🚩 Flag Format
```
L3m0nCTF{...}
```

---

## 💡 Tips for Using AI Assistance

### Good Prompts:
- "Explain this WAT instruction sequence: [paste code]"
- "What's the parity of binary number 2891?"
- "How do I hook WASM memory reads in JavaScript?"
- "Reverse this XOR encryption: [show algorithm]"

### What AI Can't Do:
- Run/execute the WASM for you
- Automatically extract the flag (that's your job!)
- Understand context without you providing the code

---

## 🔧 Useful Tools

| Tool | Purpose |
|------|---------|
| `wasm2wat` | Convert WASM binary → text |
| `wasm-objdump` | Dump WASM sections |
| Chrome DevTools | Debug WASM in browser |
| Ghidra + WASM plugin | Advanced decompilation |
| `wabt` toolkit | Full WASM toolchain |

---

## 📝 Summary

1. **Identify** the obfuscation (opaque predicates via `x()`)
2. **Understand** the parity function
3. **Trace** execution to find real code path
4. **Reverse** the input transformation
5. **Extract** the flag from encrypted data

Good luck, challenger! 🔓
