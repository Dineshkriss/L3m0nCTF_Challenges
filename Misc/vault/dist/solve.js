#!/usr/bin/env node
/**
 * VAULT BREACH - Solver Script
 * 
 * This script decrypts the flag from the vault.wasm challenge
 * by XORing the encrypted flag data with the key.
 * 
 * Data extracted from WASM memory offsets:
 * - Encrypted flag: offset 3584
 * - XOR key: offset 3700 (16 bytes, repeating)
 */

// Encrypted flag bytes from WASM data section (offset 3584)
const ENC_FLAG = [
  0x16, 0x0F, 0x12, 0x2E, 0xF3, 0x08, 0x7E, 0xCA,
  0x15, 0x7A, 0xC7, 0xD4, 0x34, 0x9E, 0x0B, 0xF2,
  0x6E, 0x4D, 0x0A, 0x2D, 0xC2, 0x3B, 0x58, 0xBF,
  0x0A, 0x3C, 0x90, 0x93, 0x2D, 0xF2, 0x48, 0xDD,
  0x6E, 0x4E, 0x4C, 0x41, 0xFB, 0x3E, 0x44, 0xF1
];

// XOR key from WASM data section (offset 3700)
const XOR_KEY = [
  0x5A, 0x3C, 0x7F, 0x1E, 0x9D, 0x4B, 0x2A, 0x8C,
  0x6E, 0x0D, 0xF3, 0xA7, 0x59, 0xC1, 0x3B, 0x82
];

console.log("=== VAULT BREACH SOLVER ===\n");
console.log("Encrypted bytes:", ENC_FLAG.length);
console.log("XOR key length:", XOR_KEY.length, "(repeating)\n");

// Decrypt: XOR each encrypted byte with the corresponding key byte
let flag = "";
for (let i = 0; i < ENC_FLAG.length; i++) {
  const decryptedByte = ENC_FLAG[i] ^ XOR_KEY[i % XOR_KEY.length];
  flag += String.fromCharCode(decryptedByte);
}

console.log("🚩 FLAG:", flag);
console.log("\n=== VERIFICATION ===");
console.log("Length:", flag.length, "(expected: 40)");
console.log("Starts with L3m0nCTF{:", flag.startsWith("L3m0nCTF{"));
console.log("Ends with }:", flag.endsWith("}"));

// Verify hash (Stage 5 check in WASM)
function mixHash(data) {
  let hash = 0x1337BEEF >>> 0;
  for (let i = 0; i < data.length; i++) {
    hash = (hash ^ (data.charCodeAt(i) << (i % 24))) >>> 0;
    hash = ((hash << 5) | (hash >>> 27)) >>> 0;
  }
  return hash;
}

const hash = mixHash(flag);
console.log("Hash & 0xFFFF:", "0x" + (hash & 0xFFFF).toString(16).toUpperCase(), "(expected: 0xD9CE)");
