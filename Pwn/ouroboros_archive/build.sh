#!/bin/bash
# Build script for Ouroboros Archive challenge

set -e

echo "[*] Building Ouroboros Archive challenge..."

# Compile unpacker
echo "[*] Compiling unzipper..."
gcc -o unzipper src/unzipper.c \
    -Wall -Wextra \
    -fstack-protector-all \
    -D_FORTIFY_SOURCE=2 \
    -lzip \
    -O2

# Strip symbols
echo "[*] Stripping symbols..."
strip --strip-all unzipper

#Move to dist
echo "[*] Moving to dist/..."
mv unzipper dist/

echo "[+] Build complete!"
echo ""
echo "Distribution files:"
echo "  - dist/unzipper"
echo ""
echo "Next steps:"
echo "  1. Test with solution/create_exploit.py"
echo "  2. Build Docker container: cd docker && docker-compose build"
echo "  3. Run locally: ./dist/unzipper test.zip"
