#!/usr/bin/env python3
"""
solve_phantom.py - Reference solution for Phantom Resolver challenge

This script demonstrates how to solve the challenge by:
1. Extracting constants from libmonitor.so (via reversing)
2. Computing the expected binary hash
3. Patching server_daemon.template to match the hash
4. Running the patched binary to trigger the backdoor
"""

import struct
import subprocess
import os
import sys

# ============== CONSTANTS (extracted from reversing libmonitor.so) ==============
XOR_KEY = bytes([0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF, 0x00])
ENCODED_HASH = bytes([0x8d, 0xed, 0x2d, 0xe0, 0x20, 0x15, 0x28, 0xe0])
MAGIC_OFFSET = 0x3000  # Actual location found in binary
MAGIC_BYTES = b"CHROMA"

# ============== FNV-1a hash function ==============
def fnv1a_hash(data):
    """Replicate the binary's compute_hash function"""
    h = 0xcbf29ce484222325
    for b in data:
        h ^= b
        h = (h * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h

# ============== Decode expected hash ==============
def decode_expected_hash():
    """Decode the XOR-obfuscated expected hash"""
    expected = 0
    for i in range(8):
        expected |= (ENCODED_HASH[i] ^ XOR_KEY[i]) << (i * 8)
    return expected

# ============== Find magic section offset ==============
def find_magic_offset(binary_data):
    """Find the PLACEHOLDER_DATA or existing magic bytes"""
    # Look for placeholder first
    placeholder = b'PLACEHOLDER_DATA'
    idx = binary_data.find(placeholder)
    if idx != -1:
        return idx
    
    # Look for CHROMA (already patched)
    idx = binary_data.find(b'CHROMA')
    if idx != -1:
        return idx
    
    # Default to 0x1000 if not found
    print("[!] Could not find magic section, using default offset 0x1000")
    return 0x1000

# ============== Main solve function ==============
def solve():
    print("=" * 60)
    print("Phantom Resolver - Automated Solver")
    print("=" * 60)
    
    # Step 1: Decode expected hash
    expected_hash = decode_expected_hash()
    print(f"\n[*] Expected binary hash: {expected_hash:016x}")
    
    # Step 2: Read template binary
    if not os.path.exists('server_daemon.template'):
        print("[!] Error: server_daemon.template not found")
        print("[!] Make sure you're in the dist/ directory")
        return False
    
    with open('server_daemon.template', 'rb') as f:
        binary = bytearray(f.read())
    
    print(f"[*] Loaded template binary ({len(binary)} bytes)")
    
    # Step 3: Find and patch magic bytes
    magic_offset = find_magic_offset(binary)
    print(f"[*] Magic section found at offset: 0x{magic_offset:x}")
    
    binary[magic_offset:magic_offset+6] = MAGIC_BYTES
    print(f"[*] Patched CHROMA magic bytes at offset 0x{magic_offset:x}")
    
    # Step 4: Brute-force patch bytes to match hash
    # The template uses a 2-byte placeholder that needs adjustment
    print("[*] Searching for correct patch bytes...")
    
    # We'll try patching at offset 0x500 (adjustable)
    PATCH_OFFSET = 0x500
    
    # For speed, we'll try a limited range first
    found = False
    for b1 in range(256):
        if found:
            break
        for b2 in range(256):
            test_binary = binary.copy()
            test_binary[PATCH_OFFSET] = b1
            test_binary[PATCH_OFFSET + 1] = b2
            
            h = fnv1a_hash(test_binary[:4096])
            if h == expected_hash:
                print(f"[+] Found matching patch: 0x{b1:02x} 0x{b2:02x}")
                binary[PATCH_OFFSET] = b1
                binary[PATCH_OFFSET + 1] = b2
                found = True
                break
        
        if b1 % 32 == 0:
            print(f"    Searching... {b1}/256", end='\r')
    
    if not found:
        print("[!] Could not find matching hash patch")
        print("[!] The template may already be correct, trying anyway...")
    
    # Step 5: Write patched binary
    output_name = 'server_daemon'
    with open(output_name, 'wb') as f:
        f.write(binary)
    
    os.chmod(output_name, 0o755)
    print(f"\n[+] Wrote patched binary: {output_name}")
    
    # Step 6: Verify hash
    with open(output_name, 'rb') as f:
        actual_hash = fnv1a_hash(f.read(4096))
    
    print(f"[*] Actual binary hash:   {actual_hash:016x}")
    print(f"[*] Expected hash:        {expected_hash:016x}")
    
    if actual_hash == expected_hash:
        print("[+] ✓ Hash matches!")
    else:
        print("[!] ✗ Hash mismatch - backdoor may not trigger")
    
    # Step 7: Run the binary with correct arguments
    print("\n[*] Running patched binary with --daemon flag...")
    print("=" * 60)
    
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = '.'
    
    try:
        result = subprocess.run(
            ['./server_daemon', '--daemon'],
            env=env,
            capture_output=True,
            text=True,
            timeout=5
        )
        
        print(result.stdout)
        
        if 'flag{' in result.stdout:
            print("=" * 60)
            print("[+] SUCCESS! Flag captured!")
            # Extract and highlight the flag
            for line in result.stdout.split('\n'):
                if 'flag{' in line:
                    print(f"[+] FLAG: {line.strip()}")
            return True
        else:
            print("[!] No flag found in output")
            return False
            
    except subprocess.TimeoutExpired:
        print("[!] Binary timed out (might be running as daemon)")
        print("[!] Try running manually: ./server_daemon --daemon")
        return False
    except Exception as e:
        print(f"[!] Error running binary: {e}")
        return False

if __name__ == '__main__':
    success = solve()
    sys.exit(0 if success else 1)
