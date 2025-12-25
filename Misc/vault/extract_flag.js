// Parity function (same as x())
function parity(n) {
    let result = 0;
    while (n !== 0) {
        result ^= n & 1;
        n >>>= 1;
    }
    return result;
}

// Load and analyze the WASM
const fs = require('fs');
const wasmBuffer = fs.readFileSync('vault.wasm');

// Parse the data section - encrypted data starts at offset 3584
// Let's try to brute force character by character with format L3m0nCTF{...}

async function analyze() {
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: {
            x: parity
        }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Read encrypted data from offset 3584
    console.log("Encrypted data at 3584:");
    console.log(Array.from(memory.slice(3584, 3650)).map(b => b.toString(16).padStart(2, '0')).join(' '));
    
    // Try brute force approach - character by character
    const known = "L3m0nCTF{";
    let flag = known;
    
    // Try each position
    for (let pos = known.length; pos < 50; pos++) {
        let found = false;
        for (let c = 32; c < 127; c++) {
            const testFlag = flag + String.fromCharCode(c);
            
            // Reset memory
            memory.fill(0, 0, 256);
            
            // Write test flag
            const encoded = new TextEncoder().encode(testFlag);
            memory.set(encoded, 0);
            memory[encoded.length] = 0;
            
            // Try unlock
            const result = unlock();
            
            if (result !== 0) {
                console.log("FLAG FOUND:", testFlag);
                process.exit(0);
            }
        }
        
        // If we found closing brace, stop
        if (flag.endsWith('}')) {
            console.log("Reached end of flag format");
            break;
        }
    }
    
    console.log("Brute force complete. Flag not found via simple approach.");
    console.log("Partial:", flag);
}

analyze().catch(console.error);
