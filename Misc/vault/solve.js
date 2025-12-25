const fs = require('fs');
const wasmBuffer = fs.readFileSync('vault.wasm');

// Parity function
function parity(n) {
    let result = 0;
    while (n !== 0) {
        result ^= n & 1;
        n >>>= 1;
    }
    return result;
}

async function solve() {
    // Track all x() calls to understand the control flow
    let callLog = [];
    
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: {
            x: (n) => {
                const result = parity(n);
                callLog.push({n, result});
                return result;
            }
        }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Test with a known format flag
    const testFlag = "L3m0nCTF{test_flag_123}";
    const encoded = new TextEncoder().encode(testFlag);
    memory.set(encoded, 0);
    memory[encoded.length] = 0;
    
    console.log("Testing with:", testFlag);
    callLog = [];
    const result = unlock();
    console.log("Result:", result);
    console.log("Number of x() calls:", callLog.length);
    
    // Print memory state after unlock to see what transformations happened
    console.log("\nMemory after unlock (first 100 bytes):");
    console.log(Array.from(memory.slice(0, 100)).map(b => b.toString(16).padStart(2, '0')).join(' '));
    
    // The encrypted data for comparison
    console.log("\nEncrypted reference data (at 3584):");
    const encData = memory.slice(3584, 3650);
    console.log(Array.from(encData).map(b => b.toString(16).padStart(2, '0')).join(' '));
    
    // Try to find any printable strings in memory
    console.log("\nSearching for printable strings in full memory...");
    const fullMem = memory.slice(0, 65536);
    let current = "";
    for (let i = 0; i < fullMem.length; i++) {
        if (fullMem[i] >= 32 && fullMem[i] < 127) {
            current += String.fromCharCode(fullMem[i]);
        } else if (current.length > 4) {
            console.log(`Offset ${i - current.length}: "${current}"`);
            current = "";
        } else {
            current = "";
        }
    }
}

solve();
