#!/bin/bash
# Build script for Phantom Resolver challenge

set -e

echo "[*] Building Phantom Resolver challenge..."

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Build the shared library
echo -e "${BLUE}[*] Compiling libmonitor.so...${NC}"
gcc -shared -fPIC -O2 src/libmonitor.c -o dist/libmonitor.so

# Strip symbols to make reversing harder
echo -e "${BLUE}[*] Stripping symbols...${NC}"
strip --strip-all dist/libmonitor.so

# Build the server daemon (full version with proper linking)
echo -e "${BLUE}[*] Compiling server_daemon (full)...${NC}"
gcc -O2 src/server_daemon.c -o server_daemon_full -L./dist -lmonitor -Wl,-rpath,.

# Calculate the hash of the compiled binary
echo -e "${BLUE}[*] Calculating binary hash...${NC}"
HASH=$(python3 << 'EOF'
import struct

def compute_hash(data):
    # FNV-1a hash
    h = 0xcbf29ce484222325
    for b in data:
        h ^= b
        h = (h * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h

with open('server_daemon_full', 'rb') as f:
    data = f.read(4096)

h = compute_hash(data)
print(f"{h:016x}")

# Also print the XOR-encoded version for verification
xor_key = bytes([0x42, 0x13, 0x37, 0xDE, 0xAD, 0xBE, 0xEF, 0x00])
encoded = bytearray()
for i in range(8):
    encoded.append(((h >> (i*8)) & 0xFF) ^ xor_key[i])

print("Encoded hash (for libmonitor.c):", ' '.join(f'0x{b:02x}' for b in encoded))
EOF
)

echo -e "${GREEN}[+] Binary hash: $HASH${NC}"

# Now create the template by copying and modifying
echo -e "${BLUE}[*] Creating server_daemon.template...${NC}"
cp server_daemon_full dist/server_daemon.template

# Write CHROMA magic bytes at offset 0x1000
# Note: The actual offset might vary, this is a simplified approach
echo -e "${BLUE}[*] Patching magic bytes into template...${NC}"
python3 << 'EOF'
# Patch CHROMA into the binary at the .magic section
with open('dist/server_daemon.template', 'r+b') as f:
    data = bytearray(f.read())
    
    # Find PLACEHOLDER_DATA and replace with CHROMA
    placeholder = b'PLACEHOLDER_DATA'
    idx = data.find(placeholder)
    
    if idx != -1:
        print(f"[*] Found placeholder at offset: 0x{idx:x}")
        # Replace with CHROMA (keep rest as padding)
        data[idx:idx+6] = b'CHROMA'
        
        f.seek(0)
        f.write(data)
        print("[+] Patched CHROMA magic bytes")
    else:
        print("[!] Warning: Could not find PLACEHOLDER_DATA")
        print("[!] The .magic section may not be at 0x1000")
        print("[!] Manually patch or adjust linker script")
EOF

# Clean up
rm -f server_daemon_full

echo -e "${GREEN}[+] Build complete!${NC}"
echo ""
echo "Distribution files:"
echo "  - dist/libmonitor.so"
echo "  - dist/server_daemon.template"
echo ""
echo "Next steps:"
echo "  1. Update encoded_hash[] in libmonitor.c if needed"
echo "  2. Rebuild if hash doesn't match"
echo "  3. Test with solution/solve.py"
