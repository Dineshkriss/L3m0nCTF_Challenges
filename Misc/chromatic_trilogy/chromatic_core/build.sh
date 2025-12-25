#!/bin/bash
# Build script for CHROMATIC CORE challenge  
# Chromatic Trilogy - Part 3

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}[*] Building CHROMATIC CORE challenge...${NC}"

mkdir -p dist

# Compile
echo -e "${BLUE}[*] Compiling chromatic binary...${NC}"
gcc -o dist/chromatic src/chromatic.c \
    -O2 \
    -Wall -Wextra \
    -fstack-protector-all \
    -D_FORTIFY_SOURCE=2 \
    -Wl,-z,relro,-z,now \
    -fPIE -pie

# Strip
echo -e "${BLUE}[*] Stripping symbols...${NC}"
strip --strip-all dist/chromatic

# Verify and compute XOR key
echo -e "${BLUE}[*] Verifying flag obfuscation...${NC}"
python3 << 'EOF'
import subprocess

# Read binary and compute key
with open('dist/chromatic', 'rb') as f:
    f.seek(0x3000)
    buffer = f.read(32)

key = 0
for b in buffer:
    key ^= b

print(f"[+] Computed XOR key: 0x{key:02x}")

# Compute encoded flag bytes
flag = "L3m0nCTF{chr0m4t1c_numb3r_1s_3_0xd34db33f}"
encoded = [ord(c) ^ key for c in flag]
print(f"[+] Flag length: {len(flag)}")
print(f"[+] Encoded bytes for source:")
for i in range(0, len(encoded), 10):
    chunk = encoded[i:i+10]
    print("        " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")

# Verify strings
result = subprocess.run(['strings', 'dist/chromatic'], capture_output=True, text=True)
if 'L3m0n' in result.stdout or 'CTF{' in result.stdout:
    print("[!] WARNING: Flag visible in strings!")
else:
    print("[+] Flag NOT visible in strings - GOOD!")
EOF

SIZE=$(stat -c%s dist/chromatic 2>/dev/null || stat -f%z dist/chromatic)

echo ""
echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║          CHROMATIC CORE BUILD COMPLETE             ║${NC}"
echo -e "${GREEN}╠════════════════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║ Binary: dist/chromatic                             ║${NC}"
echo -e "${GREEN}║ Size: ${SIZE} bytes                                 ${NC}"
echo -e "${GREEN}║                                                    ║${NC}"
echo -e "${GREEN}║ Test: ./dist/chromatic                             ║${NC}"
echo -e "${GREEN}║ Solve: python3 solution/solve.py                   ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
