#!/bin/bash
# Build script for PATHFINDER challenge
# Chromatic Trilogy - Part 2

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[*] Building PATHFINDER challenge...${NC}"

# Create dist directory
mkdir -p dist

# Compile
echo -e "${BLUE}[*] Compiling pathfinder binary...${NC}"
gcc -o dist/pathfinder src/pathfinder.c \
    -O2 \
    -Wall -Wextra \
    -fstack-protector-all \
    -D_FORTIFY_SOURCE=2 \
    -Wl,-z,relro,-z,now \
    -fPIE -pie

# Strip symbols
echo -e "${BLUE}[*] Stripping symbols...${NC}"
strip --strip-all dist/pathfinder

# Verify flag not in strings
echo -e "${BLUE}[*] Verifying flag obfuscation...${NC}"
if strings dist/pathfinder | grep -qE "L3m0n|CTF\{"; then
    echo "[!] WARNING: Flag visible in strings!"
else
    echo "[+] Flag NOT visible in strings - GOOD!"
fi

# Compute and display XOR key
python3 << 'EOF'
# Compute XOR key from magic section
with open('dist/pathfinder', 'rb') as f:
    f.seek(0x3000)
    buffer = f.read(32)

key = 0
for b in buffer:
    key ^= b

print(f"[+] Computed XOR key: 0x{key:02x}")

# Verify flag encoding
flag = "L3m0nCTF{p4th_thr0ugh_3ncrypt3d_m4z3_0xc4f3}"
encoded = [ord(c) ^ key for c in flag]
print(f"[+] Flag length: {len(flag)}")
print(f"[+] Expected encoded bytes: {', '.join(f'0x{b:02x}' for b in encoded[:10])}...")
EOF

# Get file size
SIZE=$(stat -c%s dist/pathfinder 2>/dev/null || stat -f%z dist/pathfinder)

echo ""
echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║            PATHFINDER BUILD COMPLETE               ║${NC}"
echo -e "${GREEN}╠════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║ Binary: dist/pathfinder                            ║${NC}"
echo -e "${GREEN}║ Size: ${SIZE} bytes                                 ${NC}"
echo -e "${GREEN}║                                                    ║${NC}"
echo -e "${GREEN}║ Test: ./dist/pathfinder                            ║${NC}"
echo -e "${GREEN}║ Solve: python3 solution/solve.py                   ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
