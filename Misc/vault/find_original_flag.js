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

async function findFlag() {
    let callCount = 0;
    
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: { 
            x: (n) => {
                callCount++;
                return parity(n);
            }
        }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Common CTF flag formats to try
    const prefixes = [
        "flag{", "FLAG{", "ctf{", "CTF{", 
        "picoCTF{", "PICO{", "htb{", "HTB{",
        "inctf{", "INCTF{", "ductf{", "DUCTF{",
        "wasm{", "WASM{", "vault{", "VAULT{",
        ""  // No prefix - try raw
    ];
    
    const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-!}";
    
    // Try each prefix with timing side-channel
    for (const prefix of prefixes) {
        console.log(`\nTrying prefix: "${prefix}"`);
        let flag = prefix;
        let prevCalls = 0;
        
        // Get baseline
        memory.fill(0, 0, 256);
        memory.set(new TextEncoder().encode(flag), 0);
        callCount = 0;
        unlock();
        prevCalls = callCount;
        
        for (let pos = flag.length; pos < 50; pos++) {
            let bestChar = '';
            let maxCalls = 0;
            
            for (const c of charset) {
                const testFlag = flag + c;
                memory.fill(0, 0, 256);
                memory.set(new TextEncoder().encode(testFlag), 0);
                memory[testFlag.length] = 0;
                
                callCount = 0;
                const result = unlock();
                
                if (result !== 0) {
                    console.log("\n🎉 FLAG FOUND:", testFlag);
                    process.exit(0);
                }
                
                if (callCount > maxCalls) {
                    maxCalls = callCount;
                    bestChar = c;
                }
            }
            
            flag += bestChar;
            
            if (bestChar === '}' || flag.length > 45) break;
        }
        
        console.log(`  Best guess: ${flag}`);
    }
}

findFlag();
