#!/bin/bash
# Build script for VERTEX challenge
# Chromatic Trilogy - Part 1

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}[*] Building VERTEX challenge...${NC}"

# Create dist directory
mkdir -p dist

# Compile with security hardening and obfuscation
echo -e "${BLUE}[*] Compiling vertex binary...${NC}"
gcc -o dist/vertex src/vertex.c \
    -O2 \
    -Wall -Wextra \
    -fstack-protector-all \
    -D_FORTIFY_SOURCE=2 \
    -Wl,-z,relro,-z,now \
    -fno-plt \
    -fPIE -pie

# Check if compilation succeeded
if [ ! -f dist/vertex ]; then
    echo -e "${RED}[!] Compilation failed!${NC}"
    exit 1
fi

# Strip symbols to make reversing harder
echo -e "${BLUE}[*] Stripping symbols...${NC}"
strip --strip-all dist/vertex

# Now we need to encode the flag with the correct key
echo -e "${BLUE}[*] Computing XOR key from binary...${NC}"

# Get the magic section offset and compute key
python3 << 'EOF'
import struct

# Read binary
with open('dist/vertex', 'rb') as f:
    data = f.read()

# Find the magic marker
magic = b"VERTEX_CHROMATIC_TRILOGY_KEY"
idx = data.find(magic)

if idx == -1:
    print("[!] Warning: Magic marker not found in expected location")
    print("[*] Searching for magic in binary...")
    # Try to find it anywhere
    if magic in data:
        idx = data.find(magic)
        print(f"[+] Found magic at offset: 0x{idx:x}")
    else:
        print("[!] Magic marker not in binary!")
        exit(1)
else:
    print(f"[+] Magic marker at offset: 0x{idx:x}")

# Read 32 bytes from offset 0x3000 (or wherever magic is)
offset = 0x3000
if len(data) > offset + 32:
    buffer = data[offset:offset+32]
    
    # Compute XOR key
    key = 0
    for b in buffer:
        key ^= b
    
    print(f"[+] Computed XOR key: 0x{key:02x}")
    
    # Encode the flag
    flag = "L3m0nCTF{7h3_v3rt1c3s_4r3_c0nn3ct3d_0x7f2a}"
    encoded = []
    for c in flag:
        encoded.append(ord(c) ^ key)
    
    print(f"[+] Flag length: {len(flag)}")
    print("[+] Encoded flag bytes:")
    print("    " + ", ".join(f"0x{b:02x}" for b in encoded))
else:
    print(f"[!] Binary too small for offset 0x{offset:x}")
    
# Verify strings cannot find flag
import subprocess
result = subprocess.run(['strings', 'dist/vertex'], capture_output=True, text=True)
if 'L3m0n' in result.stdout or 'CTF{' in result.stdout:
    print("[!] WARNING: Flag visible in strings!")
else:
    print("[+] Flag NOT visible in strings output - good!")

EOF

# Get file size
SIZE=$(stat -c%s dist/vertex 2>/dev/null || stat -f%z dist/vertex)

echo ""
echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║              VERTEX BUILD COMPLETE                 ║${NC}"
echo -e "${GREEN}╠════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║ Binary: dist/vertex                                ║${NC}"
echo -e "${GREEN}║ Size: ${SIZE} bytes                                 ${NC}"
echo -e "${GREEN}║                                                    ║${NC}"
echo -e "${GREEN}║ Test: ./dist/vertex                                ║${NC}"
echo -e "${GREEN}║ Solve: python3 solution/solve.py                   ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
