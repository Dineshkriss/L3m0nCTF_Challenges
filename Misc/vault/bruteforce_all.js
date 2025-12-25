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

async function bruteforce() {
    const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
        env: { x: parity }
    });
    
    const memory = new Uint8Array(wasmModule.instance.exports.memory.buffer);
    const unlock = wasmModule.instance.exports.unlock;
    
    // Try 4-digit PINs first (BambooFox style)
    console.log("Trying 4-digit PINs...");
    for (let i = 0; i <= 9999; i++) {
        const pin = i.toString().padStart(4, '0');
        memory.fill(0, 0, 256);
        memory.set(new TextEncoder().encode(pin), 0);
        if (unlock() !== 0) {
            console.log("FLAG (4-digit):", pin);
            return;
        }
    }
    console.log("No 4-digit PIN found.");
    
    // Try common flag formats with short content
    const prefixes = ["flag{", "FLAG{", "wctf{", "actf{", "CTF{"];
    const chars = "abcdefghijklmnopqrstuvwxyz0123456789_";
    
    console.log("Trying short flags...");
    for (const prefix of prefixes) {
        // Try 1-4 char content
        for (let len = 1; len <= 3; len++) {
            const max = Math.pow(chars.length, len);
            for (let i = 0; i < max; i++) {
                let content = "";
                let n = i;
                for (let j = 0; j < len; j++) {
                    content += chars[n % chars.length];
                    n = Math.floor(n / chars.length);
                }
                const flag = prefix + content + "}";
                memory.fill(0, 0, 256);
                memory.set(new TextEncoder().encode(flag), 0);
                if (unlock() !== 0) {
                    console.log("FLAG:", flag);
                    return;
                }
            }
        }
        console.log(`Checked prefix: ${prefix}`);
    }
    
    console.log("Short flag bruteforce complete, no match found.");
}

bruteforce();
