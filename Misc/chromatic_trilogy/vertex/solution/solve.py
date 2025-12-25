#!/usr/bin/env python3
"""
VERTEX Challenge Solver
Chromatic Trilogy - Part 1

This solver demonstrates how to extract and solve the bipartite graph challenge
by analyzing the binary at runtime using GDB or by understanding the algorithm.
"""

import subprocess
import struct
import os
import sys

def compute_binary_hash(binary_path):
    """Compute the same hash the binary uses to generate the graph"""
    with open(binary_path, 'rb') as f:
        data = f.read()
    
    # FNV-1a hash (matching the C implementation)
    hash_val = 0xcbf29ce484222325
    for b in data:
        hash_val ^= b
        hash_val = (hash_val * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    
    # Spread hash across 32 bytes
    output = []
    for i in range(32):
        byte_val = ((hash_val >> ((i % 8) * 8)) ^ (hash_val >> (7 - (i % 8)))) & 0xFF
        output.append(byte_val)
        hash_val = (hash_val * 0x5851f42d4c957f2d + byte_val) & 0xFFFFFFFFFFFFFFFF
    
    return bytes(output)

def generate_graph(binary_hash, num_vertices=16):
    """Generate the same graph the binary generates"""
    adj_matrix = [[0] * num_vertices for _ in range(num_vertices)]
    
    # Bipartite: Set A = 0-7, Set B = 8-15
    for i in range(8):
        for j in range(8, num_vertices):
            hash_idx = ((i * j) + (i ^ j)) % 32
            edge = (binary_hash[hash_idx] ^ binary_hash[i] ^ binary_hash[j % 32]) & 1
            
            # Ensure minimum connectivity (matching C code)
            if j == i + 8:
                edge = 1
            
            adj_matrix[i][j] = edge
            adj_matrix[j][i] = edge
    
    # Add mandatory edges (matching C code)
    adj_matrix[0][8] = adj_matrix[8][0] = 1
    adj_matrix[3][11] = adj_matrix[11][3] = 1
    adj_matrix[7][15] = adj_matrix[15][7] = 1
    
    return adj_matrix

def verify_bipartite(adj_matrix, set_a, set_b):
    """Verify that the partition is valid for a bipartite graph"""
    # No edges within set A
    for i in set_a:
        for j in set_a:
            if i != j and adj_matrix[i][j]:
                return False
    
    # No edges within set B
    for i in set_b:
        for j in set_b:
            if i != j and adj_matrix[i][j]:
                return False
    
    return True

def solve(binary_path):
    """Main solver"""
    print("=" * 60)
    print("VERTEX Challenge Solver")
    print("=" * 60)
    
    # Step 1: Compute binary hash
    print("\n[1] Computing binary hash...")
    binary_hash = compute_binary_hash(binary_path)
    print(f"    Hash: {binary_hash[:8].hex()}...")
    
    # Step 2: Generate graph
    print("\n[2] Generating graph from hash...")
    adj_matrix = generate_graph(binary_hash)
    
    # Print adjacency matrix
    print("\n    Adjacency Matrix:")
    for i, row in enumerate(adj_matrix):
        print(f"    {i:2d}: {' '.join(str(x) for x in row)}")
    
    # Step 3: The graph is bipartite by construction
    # Set A = vertices 0-7, Set B = vertices 8-15
    set_a = list(range(8))
    set_b = list(range(8, 16))
    
    print("\n[3] Verifying bipartite partition...")
    print(f"    Set A: {set_a}")
    print(f"    Set B: {set_b}")
    
    if verify_bipartite(adj_matrix, set_a, set_b):
        print("    ✓ Valid bipartite partition!")
    else:
        print("    ✗ Invalid partition - debugging needed")
        return None
    
    # Step 4: Format answer
    partition = ",".join(map(str, set_a)) + ":" + ",".join(map(str, set_b))
    print(f"\n[4] Partition string: {partition}")
    
    # Step 5: Submit to binary
    print("\n[5] Submitting solution to binary...")
    try:
        result = subprocess.run(
            [binary_path, partition],
            capture_output=True,
            text=True,
            timeout=10
        )
        print(result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        return result.stdout
    except FileNotFoundError:
        print(f"    Binary not found: {binary_path}")
        print(f"    Run with correct path to binary")
        return None
    except Exception as e:
        print(f"    Error running binary: {e}")
        return None

def main():
    # Find binary
    script_dir = os.path.dirname(os.path.abspath(__file__))
    binary_path = os.path.join(script_dir, "..", "dist", "vertex")
    
    if len(sys.argv) > 1:
        binary_path = sys.argv[1]
    
    if not os.path.exists(binary_path):
        print(f"Binary not found: {binary_path}")
        print("Usage: python3 solve.py [path_to_binary]")
        print("\nBuild the binary first with: ./build.sh")
        return 1
    
    solve(binary_path)
    return 0

if __name__ == "__main__":
    sys.exit(main())
