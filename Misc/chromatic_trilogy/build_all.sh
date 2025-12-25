#!/bin/bash
# Chromatic Trilogy - Build All Challenges
# Builds all 3 challenges in the trilogy

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "╔════════════════════════════════════════════════════╗"
echo "║          CHROMATIC TRILOGY BUILD SYSTEM            ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Build VERTEX
echo "━━━ Building Challenge 1: VERTEX ━━━"
cd vertex && chmod +x build.sh && ./build.sh
cd "$SCRIPT_DIR"
echo ""

# Build PATHFINDER
echo "━━━ Building Challenge 2: PATHFINDER ━━━"
cd pathfinder && chmod +x build.sh && ./build.sh
cd "$SCRIPT_DIR"
echo ""

# Build CHROMATIC CORE  
echo "━━━ Building Challenge 3: CHROMATIC CORE ━━━"
cd chromatic_core && chmod +x build.sh && ./build.sh
cd "$SCRIPT_DIR"
echo ""

# Summary
echo "╔════════════════════════════════════════════════════╗"
echo "║              ALL BUILDS COMPLETE                   ║"
echo "╠════════════════════════════════════════════════════╣"
echo "║ Challenge 1: vertex/dist/vertex                    ║"
echo "║ Challenge 2: pathfinder/dist/pathfinder            ║"
echo "║ Challenge 3: chromatic_core/dist/chromatic         ║"
echo "╠════════════════════════════════════════════════════╣"
echo "║ Verify: strings <binary> | grep L3m0n              ║"
echo "║         (Should find nothing!)                     ║"
echo "╚════════════════════════════════════════════════════╝"
