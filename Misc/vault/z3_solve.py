#!/usr/bin/env python3
"""
Attempt to solve the WASM vault using Z3 symbolic execution
"""

# First, let's look at the encrypted data pattern
enc_data = bytes.fromhex("e8c6660cd7c1c7649d111cbe127558ca6e004e4c452da44689 8cd56535bb9bc2cbeb3630b5902aaa3544d1dcbab805615afdf96b6fcb5b7e5adabef4b60feb170545b047f34a17f37111da5a2b86ea79eb1aa2ec17a10b83796dd4f3df965b57417f4ee7".replace(" ", ""))

print("Encrypted data length:", len(enc_data))
print("Enc bytes:", enc_data.hex())

# Try to find common flag prefixes
prefixes = [b"wctf{", b"flag{", b"CTF{", b"actf{", b"pctf{", b"dice{", b"zer0pts{", b"uiuctf{"]

for prefix in prefixes:
    # XOR first bytes of enc_data with prefix to find potential key
    if len(prefix) <= len(enc_data):
        key_guess = bytes([enc_data[i] ^ prefix[i] for i in range(len(prefix))])
        print(f"If prefix is {prefix}: key starts with {key_guess.hex()}")
        
        # Check if key is simple pattern
        if len(set(key_guess)) == 1:
            print(f"  -> Single byte key: 0x{key_guess[0]:02x}")
            # Try full decryption
            full_dec = bytes([b ^ key_guess[0] for b in enc_data[:50]])
            print(f"  -> Decrypted: {full_dec}")
        
        # Check if key could be incrementing
        diffs = [key_guess[i+1] - key_guess[i] for i in range(len(key_guess)-1)]
        if len(set(diffs)) == 1:
            print(f"  -> Incrementing key with step {diffs[0]}")
