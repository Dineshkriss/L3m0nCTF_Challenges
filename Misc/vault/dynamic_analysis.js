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

async function dynamicAnalysis() {
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: { x: parity }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Read the encrypted data at offset 3584
    console.log("=== ENCRYPTED DATA AT OFFSET 3584 ===");
    const encData = memory.slice(3584, 3700);
    console.log("Hex:", Array.from(encData.slice(0, 64)).map(b => b.toString(16).padStart(2, '0')).join(' '));
    console.log("First 4 bytes (little-endian u32):", encData[0] | (encData[1] << 8) | (encData[2] << 16) | (encData[3] << 24));
    
    // The first 4 bytes look like a length: 0xb5 0x03 0x00 0x00 = 949 in little endian
    const flagLen = encData[0] | (encData[1] << 8);
    console.log("Possible flag length:", flagLen);
    
    // Let's try different XOR keys on the encrypted data
    console.log("\n=== TRYING XOR DECRYPTION ===");
    for (let key = 0; key < 256; key++) {
        const decrypted = Array.from(encData.slice(4, 40)).map(b => b ^ key);
        const str = String.fromCharCode(...decrypted);
        // Check if it looks like printable ASCII
        if (decrypted.every(b => b >= 32 && b < 127)) {
            console.log(`Key 0x${key.toString(16)}: ${str}`);
        }
    }
    
    // Try to find patterns - test with known input and see memory changes
    console.log("\n=== MEMORY DUMP BEFORE/AFTER UNLOCK ===");
    
    const testInputs = ["AAAA", "flag", "test"];
    for (const input of testInputs) {
        // Reset memory region
        memory.fill(0, 0, 512);
        memory.fill(0, 512, 1024);
        
        // Set input
        const encoded = new TextEncoder().encode(input);
        memory.set(encoded, 0);
        memory[encoded.length] = 0;
        
        console.log(`\nInput: "${input}"`);
        console.log("Memory 0-32 before:", Array.from(memory.slice(0, 32)).map(b => b.toString(16).padStart(2, '0')).join(' '));
        
        unlock();
        
        console.log("Memory 0-32 after: ", Array.from(memory.slice(0, 32)).map(b => b.toString(16).padStart(2, '0')).join(' '));
        console.log("Memory 512-544:    ", Array.from(memory.slice(512, 544)).map(b => b.toString(16).padStart(2, '0')).join(' '));
    }
    
    // Try to understand the transformation by testing single chars
    console.log("\n=== CHARACTER TRANSFORMATION ANALYSIS ===");
    const results = {};
    for (let c = 32; c < 127; c++) {
        memory.fill(0, 0, 256);
        memory[0] = c;
        memory[1] = 0;
        unlock();
        results[String.fromCharCode(c)] = memory[0];
    }
    
    // Find if it's a simple substitution
    console.log("Single char transformations (input -> output):");
    const transformed = Object.entries(results).slice(0, 20);
    console.log(transformed.map(([k, v]) => `'${k}'->0x${v.toString(16)}`).join(', '));
}

dynamicAnalysis();
