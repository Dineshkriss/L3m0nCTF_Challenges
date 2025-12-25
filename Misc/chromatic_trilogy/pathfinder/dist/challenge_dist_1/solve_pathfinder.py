#!/usr/bin/env python3
"""
PATHFINDER Challenge Solver - Chromatic Trilogy Part 2

This script demonstrates the reverse engineering process:
1. Decode the unlock key check
2. Extract forbidden nodes from .constraints section
3. Extract weight matrix from .rodata
4. Find shortest path using Dijkstra (avoiding forbidden nodes)
"""

import heapq

print("=" * 60)
print("PATHFINDER Reverse Engineering Solution")
print("=" * 60)

# ==============================================================
# Step 1: Finding the unlock key
# ==============================================================
print("\n[Step 1] Decoding the unlock key...")

# Found at offset 0x2870 in binary: 40 56 56 57 4c 52 00
obf_unlock = [0x40, 0x56, 0x56, 0x57, 0x4c, 0x52]
OBFUSCATION_KEY = 0x13  # Found by analyzing verify_unlock function

unlock_key = ""
print(f"  Obfuscated bytes: {[hex(b) for b in obf_unlock]}")
print(f"  XOR key: 0x{OBFUSCATION_KEY:02X}")
print(f"  Decoding each byte:")
for b in obf_unlock:
    decoded = chr(b ^ OBFUSCATION_KEY)
    unlock_key += decoded
    print(f"    0x{b:02X} ^ 0x{OBFUSCATION_KEY:02X} = 0x{b ^ OBFUSCATION_KEY:02X} = '{decoded}'")

print(f"\n  ✓ Unlock key: \"{unlock_key}\"")

# ==============================================================
# Step 2: Finding forbidden nodes
# ==============================================================
print("\n[Step 2] Extracting forbidden nodes from .constraints section...")

# Found at offset 0x28a0: 02 00 00 00 07 00 00 00 ff ff ff ff
# These are 32-bit integers: 2, 7, -1 (terminator)
forbidden_raw = [0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff]
print(f"  Raw bytes from 0x28a0: {[hex(b) for b in forbidden_raw]}")

forbidden_nodes = []
# Parse as 32-bit little-endian integers
i = 0
while i < len(forbidden_raw):
    value = forbidden_raw[i] | (forbidden_raw[i+1] << 8) | (forbidden_raw[i+2] << 16) | (forbidden_raw[i+3] << 24)
    if value == 0xffffffff:  # -1 terminator (signed)
        break
    forbidden_nodes.append(value)
    i += 4

print(f"  ✓ Forbidden nodes: {forbidden_nodes}")

# Alternative: The is_forbidden function uses obfuscated check
print("\n  (Alternative: Reverse is_forbidden function)")
print("    Code: return (node ^ 0x05 == 7) || (node ^ 0x05 == 2)")
print("    Solving: node ^ 5 = 7  →  node = 2")
print("    Solving: node ^ 5 = 2  →  node = 7")

# ==============================================================
# Step 3: Extracting the weight matrix
# ==============================================================
print("\n[Step 3] Extracting weight matrix from binary...")

# Weight matrix found at offset 0x26e0-0x2870 (400 bytes = 10x10 x 4 bytes)
# Reading from actual source: original_weights[10][10]
weights = [
    #   0   1   2   3   4   5   6   7   8   9
    [   0,  4,  2,  0,  0,  0,  0,  0,  0,  0 ],  # 0
    [   4,  0,  3,  5,  0,  0,  0,  0,  0,  0 ],  # 1
    [   2,  3,  0,  1,  6,  0,  0,  0,  0,  0 ],  # 2 (FORBIDDEN)
    [   0,  5,  1,  0,  2,  7,  0,  0,  0,  0 ],  # 3
    [   0,  0,  6,  2,  0,  3,  4,  0,  0,  0 ],  # 4
    [   0,  0,  0,  7,  3,  0,  2,  5,  0,  0 ],  # 5
    [   0,  0,  0,  0,  4,  2,  0,  3,  6,  0 ],  # 6
    [   0,  0,  0,  0,  0,  5,  3,  0,  2,  8 ],  # 7 (FORBIDDEN)
    [   0,  0,  0,  0,  0,  0,  6,  2,  0,  3 ],  # 8
    [   0,  0,  0,  0,  0,  0,  0,  8,  3,  0 ]   # 9
]

print("  Weight matrix (10x10):")
print("       " + " ".join(f"{i:2d}" for i in range(10)))
print("      " + "-" * 30)
for i, row in enumerate(weights):
    mark = " (FORBIDDEN)" if i in forbidden_nodes else ""
    print(f"  {i:2d} | " + " ".join(f"{w:2d}" for w in row) + mark)

# ==============================================================
# Step 4: Find shortest path using Dijkstra
# ==============================================================
print("\n[Step 4] Finding shortest path from 0 to 9 (avoiding forbidden nodes)...")

def dijkstra(weights, forbidden, start, end):
    n = len(weights)
    dist = [float('inf')] * n
    prev = [-1] * n
    dist[start] = 0
    pq = [(0, start)]
    
    while pq:
        d, u = heapq.heappop(pq)
        if u in forbidden:
            continue
        if d > dist[u]:
            continue
        for v in range(n):
            if weights[u][v] > 0 and v not in forbidden:
                new_dist = d + weights[u][v]
                if new_dist < dist[v]:
                    dist[v] = new_dist
                    prev[v] = u
                    heapq.heappush(pq, (new_dist, v))
    
    # Reconstruct path
    path = []
    node = end
    while node != -1:
        path.append(node)
        node = prev[node]
    path.reverse()
    
    return path, dist[end]

path, cost = dijkstra(weights, set(forbidden_nodes), 0, 9)

print(f"\n  Running Dijkstra...")
print(f"  Avoiding nodes: {forbidden_nodes}")
print(f"\n  ✓ Optimal path: {path}")
print(f"  ✓ Total cost: {cost}")

# Format answer
answer = ",".join(map(str, path))
print(f"\n  Answer format: \"{answer}\"")

# ==============================================================
# Step 5: Verify path manually
# ==============================================================
print("\n[Step 5] Verifying path manually...")
total = 0
print(f"  Path: ", end="")
for i in range(len(path)):
    if i > 0:
        edge_cost = weights[path[i-1]][path[i]]
        total += edge_cost
        print(f" --({edge_cost})--> ", end="")
    print(f"{path[i]}", end="")
print(f"\n  Total: {total}")

# ==============================================================
# Summary
# ==============================================================
print("\n" + "=" * 60)
print("SUMMARY")
print("=" * 60)
print(f"  Unlock key:     {unlock_key}")
print(f"  Forbidden:      {forbidden_nodes}")
print(f"  Optimal path:   {path}")
print(f"  Path cost:      {cost}")
print(f"\n  Command: ./pathfinder {unlock_key} \"{answer}\"")
print("=" * 60)
