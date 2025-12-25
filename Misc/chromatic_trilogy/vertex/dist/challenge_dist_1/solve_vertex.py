#!/usr/bin/env python3
"""
VERTEX Challenge Solver - Chromatic Trilogy Part 1

This script demonstrates the reverse engineering process:
1. Extract magic marker and derive XOR key
2. Decrypt the adjacency matrix
3. Check if graph is bipartite using BFS
4. Find the two partition sets
"""

# Step 1: Derive XOR key from magic marker
# Found in binary: "VERTEX_V2_CHROMATIC_HARDENED"
magic_marker = "VERTEX_V2_CHROMATIC_HARDENED"
key = 0
for c in magic_marker[:16]:  # Only first 16 bytes used
    key ^= ord(c)
print(f"Step 1: Derived XOR key from magic marker")
print(f"  Magic marker: '{magic_marker[:16]}'")
print(f"  XOR key: 0x{key:02x} ({key})")
print()

# Step 2: Encrypted adjacency matrix extracted from .graph_data section
# Using xxd -s 0x2540 -l 0x90 vertex
encrypted_adj = [
    [0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76],  # Row 0
    [0x77, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x77, 0x77, 0x76, 0x76, 0x77],  # Row 1
    [0x76, 0x76, 0x76, 0x77, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76],  # Row 2
    [0x76, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77],  # Row 3
    [0x76, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76],  # Row 4
    [0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76],  # Row 5
    [0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x76, 0x76],  # Row 6
    [0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76],  # Row 7
    [0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x76],  # Row 8
    [0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x77, 0x76, 0x76, 0x76],  # Row 9
    [0x77, 0x76, 0x77, 0x76, 0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x77],  # Row 10
    [0x76, 0x77, 0x76, 0x77, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x77, 0x76],  # Row 11
]

# Step 3: Decrypt adjacency matrix
n = 12
adj_matrix = [[encrypted_adj[i][j] ^ key for j in range(n)] for i in range(n)]

print(f"Step 2: Decrypted adjacency matrix (12x12):")
print("     " + " ".join(f"{i:2d}" for i in range(n)))
print("    " + "-" * 36)
for i in range(n):
    row_str = " ".join(f"{adj_matrix[i][j]:2d}" for j in range(n))
    print(f" {i:2d}| {row_str}")
print()

# Step 4: Show graph edges
print("Step 3: Graph edges:")
for i in range(n):
    neighbors = [j for j in range(n) if adj_matrix[i][j] == 1]
    print(f"  Node {i:2d}: connects to {neighbors}")
print()

# Step 5: Check if bipartite using BFS
def is_bipartite(adj, n):
    """Check if graph is bipartite and return the two partition sets"""
    color = [-1] * n
    color[0] = 0
    queue = [0]
    
    while queue:
        u = queue.pop(0)
        for v in range(n):
            if adj[u][v]:  # There's an edge
                if color[v] == -1:
                    color[v] = 1 - color[u]  # Alternate color
                    queue.append(v)
                elif color[v] == color[u]:
                    return None  # Not bipartite!
    
    set_a = sorted([i for i in range(n) if color[i] == 0])
    set_b = sorted([i for i in range(n) if color[i] == 1])
    return set_a, set_b

print("Step 4: Checking if graph is bipartite using BFS...")
result = is_bipartite(adj_matrix, n)

if result:
    set_a, set_b = result
    print(f"  ✓ Graph IS bipartite!")
    print(f"  Set A: {{{', '.join(map(str, set_a))}}}")
    print(f"  Set B: {{{', '.join(map(str, set_b))}}}")
    print()
    
    # Step 6: Form the answer
    answer = f"{','.join(map(str, set_a))}:{','.join(map(str, set_b))}"
    print(f"Step 5: Constructed answer in required format:")
    print(f"  Answer: \"{answer}\"")
    print()
    print(f"Step 6: Run the binary with this answer:")
    print(f"  ./vertex \"{answer}\"")
else:
    print("  ✗ Graph is NOT bipartite!")
