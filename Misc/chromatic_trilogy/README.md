# Chromatic Trilogy CTF Challenge

A 3-part interconnected graph theory CTF challenge series with progressive difficulty and anti-AI measures.

## Overview

| # | Challenge | Concept | Difficulty | Flag |
|---|-----------|---------|------------|------|
| 1 | **VERTEX** | Bipartite graph detection | Medium | `L3m0nCTF{7h3_v3rt1c3s_4r3_c0nn3ct3d_0x7f2a}` |
| 2 | **PATHFINDER** | Shortest path with constraints | Hard | `L3m0nCTF{p4th_thr0ugh_3ncrypt3d_m4z3_0xc4f3}` |
| 3 | **CHROMATIC CORE** | Graph 3-coloring | Very Hard | `L3m0nCTF{chr0m4t1c_numb3r_1s_3_0xd34db33f}` |

## Building

```bash
chmod +x build_all.sh
./build_all.sh
```

Individual challenges can be built separately:
```bash
cd vertex && ./build.sh
cd pathfinder && ./build.sh  
cd chromatic_core && ./build.sh
```

## Challenge Details

### 1. VERTEX

**Concept**: Binary generates a graph from its own hash. Player must identify if bipartite and provide valid 2-partition.

**Anti-AI Measures**:
- Graph generated at runtime from binary hash
- Requires debugging to extract adjacency matrix
- Answer format requires understanding memory layout

**Solution Approach**:
1. Use GDB to extract `adj_matrix` at runtime
2. Verify bipartite structure (BFS/DFS 2-coloring)
3. Submit partition in format: `0,1,2,3,4,5,6,7:8,9,10,11,12,13,14,15`

### 2. PATHFINDER

**Concept**: Find shortest path in weighted graph while avoiding hidden forbidden nodes.

**Anti-AI Measures**:
- Forbidden nodes constraint hidden in binary
- Requires finding the `forbidden_nodes` array
- Must use Dijkstra avoiding specific nodes

**Solution Approach**:
1. Extract graph from displayed adjacency matrix
2. Find forbidden nodes (3, 6) via RE
3. Run constrained Dijkstra: `0 → 2 → 5 → 7 → 9 → 11`

### 3. CHROMATIC CORE

**Concept**: Find valid 3-coloring for a 24-node graph.

**Anti-AI Measures**:
- Large graph (72 edges) exceeds context windows
- Must understand chromatic number concept
- Requires greedy/backtracking or SAT solver

**Solution Approach**:
1. Extract edge list from binary
2. Recognize 3-colorable structure
3. Apply greedy coloring: `node % 3` gives color

## Security Features

All binaries use:
- XOR flag encoding with dynamic key
- Key derived from magic bytes at offset 0x3000
- `strings` cannot reveal flag
- Symbols stripped

## Testing Solutions

```bash
# VERTEX
cd vertex && python3 solution/solve.py

# PATHFINDER  
cd pathfinder && python3 solution/solve.py

# CHROMATIC CORE
cd chromatic_core && python3 solution/solve.py
```

## Directory Structure

```
chromatic_trilogy/
├── vertex/
│   ├── src/vertex.c
│   ├── dist/vertex
│   ├── solution/solve.py
│   └── build.sh
├── pathfinder/
│   ├── src/pathfinder.c
│   ├── dist/pathfinder
│   ├── solution/solve.py
│   └── build.sh
├── chromatic_core/
│   ├── src/chromatic.c
│   ├── dist/chromatic
│   ├── solution/solve.py
│   └── build.sh
├── build_all.sh
└── README.md
```
