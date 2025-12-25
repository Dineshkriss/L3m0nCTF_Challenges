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

async function timingAttack() {
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: { x: parity }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Count x() calls as a side channel
    let callCount = 0;
    const wasmModule2 = await WebAssembly.instantiate(wasmBuffer, {
        env: { 
            x: (n) => {
                callCount++;
                return parity(n);
            }
        }
    });
    const memory2 = new Uint8Array(wasmModule2.instance.exports.memory.buffer);
    const unlock2 = wasmModule2.instance.exports.unlock;
    
    const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-!@#$%^&*(){}[]";
    let flag = "L3m0nCTF{";
    
    console.log("Starting timing-based side channel attack...\n");
    
    for (let pos = flag.length; pos < 60; pos++) {
        let bestChar = '';
        let maxCalls = 0;
        
        for (const c of charset) {
            const testFlag = flag + c;
            memory2.fill(0, 0, 256);
            const encoded = new TextEncoder().encode(testFlag);
            memory2.set(encoded, 0);
            memory2[encoded.length] = 0;
            
            callCount = 0;
            unlock2();
            
            if (callCount > maxCalls) {
                maxCalls = callCount;
                bestChar = c;
            }
        }
        
        flag += bestChar;
        console.log(`Position ${pos}: '${bestChar}' (${maxCalls} calls) -> ${flag}`);
        
        // Check if we found the flag
        memory2.fill(0, 0, 256);
        const encoded = new TextEncoder().encode(flag);
        memory2.set(encoded, 0);
        memory2[encoded.length] = 0;
        if (unlock2() !== 0) {
            console.log("\n🎉 FLAG FOUND:", flag);
            return;
        }
        
        if (bestChar === '}') {
            console.log("\nReached closing brace");
            break;
        }
    }
    
    console.log("\nFinal attempt:", flag);
}

timingAttack();
