# Chromatic Trilogy - Challenger Walkthrough

> **Difficulty**: Easy | **Category**: Reverse Engineering, Graph Theory


---

## Initial Reconnaissance

So, we are given three binaries: `vertex`, `pathfinder`, and `chromatic`. Let's run them:

![alt text](image.png)

The binary tells me nothing about what kind of graph or answer format. Time to reverse engineer.

---

# Challenge 1: VERTEX

## Step 1: Static Analysis

Let's check the binary:

```bash
$ file vertex
vertex: ELF 64-bit LSB pie executable, x86-64, stripped

$ strings vertex | head -20
# Nothing useful - flags must be encoded
```

Opening in Ghidra, I search for functions and find:
- `init_graph` - Decrypts a graph
- `verify_answer` - Validates my input
- `encrypted_adj` - 12x12 matrix in `.graph_data` section

## Step 2: Understanding the Graph Decryption

In `init_graph`, I see:
```c
key = 0;
for (i = 0; i < 16; i++) {
    key ^= magic_marker[i];
}
// XOR decrypt adjacency matrix
for (i = 0; i < 12; i++) {
    for (j = 0; j < 12; j++) {
        adj_matrix[i][j] = encrypted_adj[i][j] ^ key;
    }
}
```

The magic marker is `"VERTEX_V2_CHROMA..."`. XORing first 16 bytes:
```python
marker = "VERTEX_V2_CHROMA"
key = 0
for c in marker[:16]:
    key ^= ord(c)
print(f"Key: 0x{key:02x}")  # 0x76
```

## Step 3: Extracting and Decrypting the Graph

Extracting encrypted bytes from `.graph_data` section and XORing with 0x76:

```python
# After extraction and decryption, I get adjacency matrix:
# Node 0: connects to [1, 9, 10]
# Node 1: connects to [0, 5, 7, 8, 11]
# Node 2: connects to [3, 4, 10]
# ... (12 nodes total)
```

## Step 4: Graph Analysis - Is it Bipartite?

The decrypted graph has 12 nodes. The challenge says "prove you understand its structure."

Running BFS bipartite check:
```python
def is_bipartite(adj, n):
    color = [-1] * n
    color[0] = 0
    queue = [0]
    while queue:
        u = queue.pop(0)
        for v in range(n):
            if adj[u][v]:
                if color[v] == -1:
                    color[v] = 1 - color[u]
                    queue.append(v)
                elif color[v] == color[u]:
                    return None
    set_a = [i for i in range(n) if color[i] == 0]
    set_b = [i for i in range(n) if color[i] == 1]
    return set_a, set_b
```

Result: **Graph IS bipartite!**
- Set A: {0, 2, 5, 7, 8, 11}
- Set B: {1, 3, 4, 6, 9, 10}

## Step 5: Finding Answer Format

Looking at `verify_answer` in Ghidra:
```c
// Parses input format "a,b,c:x,y,z"
colon = strchr(input, ':');
// Parses comma-separated vertices for each set
// Verifies bipartite property against adj_matrix
```

The answer format is: **comma-separated vertices in each set, separated by colon**

## Step 6: Submitting the Answer

```bash
$ ./vertex "0,2,5,7,8,11:1,3,4,6,9,10"

════════════════════════════════════════════════════
           VERTEX - CHALLENGE COMPLETE              
════════════════════════════════════════════════════
 FLAG: L3m0nCTF{v3rt3x_SEED_A_7f2a9b}
════════════════════════════════════════════════════
```

**Flag contains `SEED_A` - this is the unlock key for PATHFINDER!**

---
L3m0nCTF{v3rt3x_SEED_A_7f2a9b}
# Challenge 2: PATHFINDER

## Step 1: Unlocking the Challenge

```bash
$ ./pathfinder
This challenge is LOCKED.
Enter the unlock key from the previous challenge.

$ ./pathfinder SEED_A
Challenge UNLOCKED.
Find the shortest path in the hidden graph.
Some nodes are forbidden - discover which ones.
```

## Step 2: Analyzing Graph Structure

In Ghidra, I find `original_weights` - a 10x10 weighted adjacency matrix:
```L3m0nCTF{v3rt3x_SEED_A_7f2a9b}
Node 0: 0→1(4), 0→2(2)
Node 1: 1→0(4), 1→2(3), 1→3(5)
...
```

## Step 3: Finding Forbidden Nodes

Looking for constraints, I find `is_forbidden`:
```c
int is_forbidden(int node) {
    int check = node ^ 0x05;
    return (check == 7 || check == 2);
}
```

Solving: `node ^ 5 == 7` → `node = 2`, `node ^ 5 == 2` → `node = 7`

**Forbidden nodes: 2 and 7**

## Step 4: Finding Optimal Path

Running Dijkstra from node 0 to node 9, avoiding forbidden nodes:

```python
import heapq

def dijkstra(weights, forbidden, start, end):
    n = len(weights)
    dist = [float('inf')] * n
    dist[start] = 0
    prev = [-1] * n
    pq = [(0, start)]
    
    while pq:
        d, u = heapq.heappop(pq)
        if u in forbidden: continue
        if d > dist[u]: continue
        for v in range(n):
            if weights[u][v] > 0 and v not in forbidden:
                if d + weights[u][v] < dist[v]:
                    dist[v] = d + weights[u][v]
                    prev[v] = u
                    heapq.heappush(pq, (dist[v], v))
    
    # Reconstruct path
    path = []
    node = end
    while node != -1:
        path.append(node)
        node = prev[node]
    return path[::-1], dist[end]

# Result: path = [0, 1, 3, 4, 6, 8, 9], cost = 24
```

## Step 5: Submitting the Path

```bash
$ ./pathfinder SEED_A "0,1,3,4,6,8,9"

Path verified: cost 24 (OPTIMAL)
════════════════════════════════════════════════════
         PATHFINDER - CHALLENGE COMPLETE            
════════════════════════════════════════════════════
 FLAG: L3m0nCTF{p4th_SEED_B_c4f3b1}
════════════════════════════════════════════════════
```

**Flag contains `SEED_B` - the unlock key for CHROMATIC CORE!**

---

# Challenge 3: CHROMATIC CORE

## Step 1: Unlocking

```bash
$ ./chromatic SEED_B
Challenge UNLOCKED.
A graph is hidden in this binary.
Find it. Determine its chromatic number.
Provide a valid coloring.
The graph has 16 nodes.
```

## Step 2: Extracting Edge List

In Ghidra, I find `encrypted_edges` in `.edges` section and `derive_edge_key`:

```c
uint8_t derive_edge_key(const char* unlock) {
    uint8_t key = 0;
    for (int i = 0; unlock[i]; i++) {
        key ^= unlock[i];
    }
    return key;
}
```

XOR of "SEED_B": `0x53 ^ 0x45 ^ 0x45 ^ 0x44 ^ 0x5F ^ 0x42 = 0x0A`

Decrypting edges with key 0x0A:
```python
encrypted = [(0x0A, 0x01), (0x0B, 0x00), ...]  # from binary
edges = [(a ^ 0x0A, b ^ 0x0A) for a, b in encrypted]
# Result: (0,11), (1,10), (1,11), (2,3), (2,4), ...
```

## Step 3: Finding Valid 3-Coloring

Building the graph and running greedy coloring:

```python
def greedy_coloring(n, edges):
    adj = [set() for _ in range(n)]
    for a, b in edges:
        adj[a].add(b)
        adj[b].add(a)
    
    colors = [-1] * n
    for node in range(n):
        neighbor_colors = {colors[v] for v in adj[node] if colors[v] != -1}
        for c in range(3):  # Try 3 colors Solver/Internal Documentation (contains flags and solutions)
SOLVER_README.md
            if c not in neighbor_colors:
                colors[node] = c
                break
    return colors

# Analyzing the result, I find the color groups:
# Color 0: {0, 4, 7, 10, 13, 15}
# Color 1: {1, 3, 6, 9, 12}
# Color 2: {2, 5, 8, 11, 14}
```

The pattern is: `[0, 1, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 0]`

## Step 4: Submitting the Coloring

```bash
$ ./chromatic SEED_B "0,1,2,1,0,2,1,0,2,1,0,2,1,0,2,0"

════════════════════════════════════════════════════
        CHROMATIC CORE - CHALLENGE COMPLETE         
════════════════════════════════════════════════════
 You found a valid coloring for the hidden graph!   
 The chromatic number χ(G) = 3.                     
════════════════════════════════════════════════════
 FLAG: L3m0nCTF{chr0m4t1c_c0mpl3t3_d34db33f}

 CONGRATULATIONS! You have completed the trilogy!   
════════════════════════════════════════════════════
```

---

## Summary

| Challenge | Key Insight | Answer |
|-----------|-------------|--------|
| VERTEX | Bipartite graph partition | `0,2,5,7,8,11:1,3,4,6,9,10` |
| PATHFINDER | Forbidden nodes via XOR (2, 7) | `0,1,3,4,6,8,9` |
| CHROMATIC | 3-colorable with scrambled groups | `0,1,2,1,0,2,1,0,2,1,0,2,1,0,2,0` |

**Flags:**
- `L3m0nCTF{v3rt3x_SEED_A_7f2a9b}`
- `L3m0nCTF{p4th_SEED_B_c4f3b1}`
- `L3m0nCTF{chr0m4t1c_c0mpl3t3_d34db33f}`
