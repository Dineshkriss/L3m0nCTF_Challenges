const fs = require('fs');
const wasmBuffer = fs.readFileSync('vault.wasm');

function parity(n) {
    let result = 0;
    while (n !== 0) {
        result ^= n & 1;
        n >>>= 1;
    }
    return result;
}

async function reverseFlag() {
    // Create a modified parity function that logs calls
    let xCalls = [];
    
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: { 
            x: (n) => {
                const r = parity(n);
                xCalls.push({n, r});
                return r;
            }
        }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Let's trace memory reads by creating a Proxy-like behavior
    // First, snapshot the original memory
    const origMem = new Uint8Array(memory);
    
    console.log("=== ANALYZING ENCRYPTED DATA STRUCTURE ===");
    const encData = memory.slice(3584, 3704);
    
    // First 4 bytes are likely flag length or header
    const header = encData[0] | (encData[1] << 8) | (encData[2] << 16) | (encData[3] << 24);
    console.log("Header value:", header, "(0x" + header.toString(16) + ")");
    
    // The actual encrypted flag data starts at offset 4
    const encFlag = encData.slice(4, 104);
    console.log("Encrypted flag bytes:", Array.from(encFlag).map(b => b.toString(16).padStart(2, '0')).join(' '));
    
    // Let's try to understand the encryption by running unlock with patterns
    console.log("\n=== PATTERN ANALYSIS ===");
    
    // Try increasing character values to see if there's a pattern
    for (let len = 1; len <= 4; len++) {
        memory.fill(0, 0, 256);
        for (let i = 0; i < len; i++) {
            memory[i] = 0x41 + i; // A, B, C, D...
        }
        
        xCalls = [];
        unlock();
        
        // Check if call count varies with length
        console.log(`Length ${len}: ${xCalls.length} x() calls`);
    }
    
    // Check offsets 512+ that seem to have data after unlock
    console.log("\n=== CHECKING LOOKUP TABLE AT 512 ===");
    memory.fill(0, 0, 256);
    memory[0] = 0x41;
    memory[1] = 0;
    unlock();
    
    // This looks like a counter/index table - 16-bit values 1,2,3,4...
    const table = [];
    for (let i = 0; i < 32; i++) {
        table.push(memory[512 + i*2] | (memory[512 + i*2 + 1] << 8));
    }
    console.log("Table values:", table.join(', '));
    
    // Let's try to find where the comparison happens by checking if the flag is related to parity
    console.log("\n=== CHECKING PARITY-BASED DECRYPTION ===");
    
    // For each constant that x() is called with, compute its parity
    // The encrypted data might be xor'd with these parities
    const paritySeq = [];
    for (let i = 0; i < 100; i++) {
        paritySeq.push(parity(i));
    }
    
    // Try XOR decryption with different patterns
    const tryDecrypt = (data, key) => {
        return Array.from(data).map((b, i) => b ^ key[i % key.length]);
    };
    
    // Try simple single-byte XOR
    console.log("\n=== BRUTE FORCE XOR KEYS ===");
    for (let key = 1; key < 256; key++) {
        const dec = encFlag.slice(0, 32).map(b => b ^ key);
        const str = String.fromCharCode(...dec);
        if (/^[a-zA-Z0-9_{}\-!]+$/.test(str) && str.length > 5) {
            console.log(`Key ${key} (0x${key.toString(16)}): "${str}"`);
        }
    }
    
    // Check for flag patterns in full memory
    console.log("\n=== SEARCHING FULL MEMORY FOR STRINGS ===");
    for (let offset = 0; offset < 65536 - 32; offset++) {
        const chunk = memory.slice(offset, offset + 32);
        const str = String.fromCharCode(...chunk);
        if (str.includes("flag") || str.includes("CTF") || str.includes("wctf") || str.includes("{")) {
            if (/^[\x20-\x7e]+$/.test(str)) {
                console.log(`Offset ${offset}: "${str.replace(/\0/g, '·')}"`);
            }
        }
    }
}

reverseFlag();
