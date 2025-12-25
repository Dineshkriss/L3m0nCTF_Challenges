#!/usr/bin/env python3
# Quick test script to check if magic bytes are correct

XOR_KEY = 0x42

print("Expected CHROMA bytes after XOR:")
expected = [0x01, 0x2a, 0x30, 0x2d, 0x29, 0x03]
decoded = bytes([b ^ XOR_KEY for b in expected])
print(f"  Hex: {decoded.hex()}")
print(f"  ASCII: {decoded}")

print("\nChecking binary:")
with open('dist/server_daemon', 'rb') as f:
    f.seek(0x3000)
    actual = f.read(6)
    print(f"  Found at 0x3000: {actual}")
    print(f"  Match: {actual == decoded}")
