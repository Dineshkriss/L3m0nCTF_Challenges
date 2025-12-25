#!/usr/bin/env python3
"""
CHROMATIC CORE Challenge Solver
Chromatic Trilogy - Part 3

This solver demonstrates how to analyze the graph structure
and find a valid 3-coloring.
"""

import subprocess
import os
import sys

NUM_NODES = 24

# Graph edges (from binary analysis)
# 3-colorable graph with 3 independent sets:
# Set A (color 0): 0, 3, 6, 9, 12, 15, 18, 21
# Set B (color 1): 1, 4, 7, 10, 13, 16, 19, 22
# Set C (color 2): 2, 5, 8, 11, 14, 17, 20, 23
EDGES = [
    # Between Set A and Set B
    (0, 1), (0, 4), (0, 7),
    (3, 1), (3, 4), (3, 10),
    (6, 4), (6, 7), (6, 13),
    (9, 7), (9, 10), (9, 16),
    (12, 10), (12, 13), (12, 19),
    (15, 13), (15, 16), (15, 22),
    (18, 16), (18, 19), (18, 1),
    (21, 19), (21, 22), (21, 4),
    
    # Between Set A and Set C
    (0, 2), (0, 5), (0, 8),
    (3, 2), (3, 5), (3, 11),
    (6, 5), (6, 8), (6, 14),
    (9, 8), (9, 11), (9, 17),
    (12, 11), (12, 14), (12, 20),
    (15, 14), (15, 17), (15, 23),
    (18, 17), (18, 20), (18, 2),
    (21, 20), (21, 23), (21, 5),
    
    # Between Set B and Set C
    (1, 2), (1, 5), (1, 8),
    (4, 5), (4, 8), (4, 11),
    (7, 8), (7, 11), (7, 14),
    (10, 11), (10, 14), (10, 17),
    (13, 14), (13, 17), (13, 20),
    (16, 17), (16, 20), (16, 23),
    (19, 20), (19, 23), (19, 2),
    (22, 23), (22, 2), (22, 5),
]

def build_adjacency_list():
    """Build adjacency list from edges"""
    adj = {i: set() for i in range(NUM_NODES)}
    for u, v in EDGES:
        adj[u].add(v)
        adj[v].add(u)
    return adj

def greedy_coloring(adj):
    """Use greedy algorithm to find a valid coloring"""
    colors = [-1] * NUM_NODES
    
    for node in range(NUM_NODES):
        # Find colors used by neighbors
        used_colors = set()
        for neighbor in adj[node]:
            if colors[neighbor] != -1:
                used_colors.add(colors[neighbor])
        
        # Assign smallest available color
        for c in range(3):  # Try colors 0, 1, 2
            if c not in used_colors:
                colors[node] = c
                break
        
        # Fallback if all 3 colors used (shouldn't happen for 3-colorable graph)
        if colors[node] == -1:
            colors[node] = 0
    
    return colors

def layer_based_coloring():
    """Use proper greedy coloring respecting all edges"""
    adj = build_adjacency_list()
    colors = [None] * NUM_NODES
    
    for node in range(NUM_NODES):
        # Find colors used by already-colored neighbors
        used = set()
        for neighbor in adj[node]:
            if colors[neighbor] is not None:
                used.add(colors[neighbor])
        
        # Assign first available color
        for c in range(3):
            if c not in used:
                colors[node] = c
                break
        
        if colors[node] is None:
            colors[node] = 0  # Fallback, shouldn't happen
    
    return colors

def verify_coloring(colors, adj):
    """Verify the coloring is valid"""
    for u, v in EDGES:
        if colors[u] == colors[v]:
            return False, (u, v)
    return True, None

def solve(binary_path):
    """Main solver"""
    print("=" * 60)
    print("CHROMATIC CORE Challenge Solver")
    print("=" * 60)
    
    # Step 1: Analyze graph
    print("\n[1] Analyzing graph structure...")
    adj = build_adjacency_list()
    print(f"    Nodes: {NUM_NODES}")
    print(f"    Edges: {len(EDGES)}")
    
    # Step 2: Recognize layer structure
    print("\n[2] Recognizing layer structure:")
    print("    Layer 0 (nodes 0-7): Connect to Layer 1 + some intra-layer")
    print("    Layer 1 (nodes 8-15): Connect to Layers 0 and 2 + some intra-layer")
    print("    Layer 2 (nodes 16-23): Connect to Layer 1 + some intra-layer")
    print("    → Using greedy coloring algorithm")
    
    # Step 3: Compute coloring
    print("\n[3] Computing coloring using greedy algorithm...")
    colors = layer_based_coloring()
    
    # Verify
    valid, conflict = verify_coloring(colors, adj)
    
    if not valid:
        print(f"    ✗ Greedy also failed at edge {conflict}")
        return None
    
    print("    ✓ Valid 3-coloring found!")
    print(f"    Colors: {colors}")
    
    # Step 4: Format answer
    coloring_str = ",".join(map(str, colors))
    print(f"\n[4] Coloring string: {coloring_str[:50]}...")
    
    # Step 5: Submit
    print("\n[5] Submitting solution to binary...")
    try:
        result = subprocess.run(
            [binary_path, coloring_str],
            capture_output=True,
            text=True,
            timeout=10
        )
        print(result.stdout)
        return result.stdout
    except FileNotFoundError:
        print(f"    Binary not found: {binary_path}")
        return None
    except Exception as e:
        print(f"    Error: {e}")
        return None

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    binary_path = os.path.join(script_dir, "..", "dist", "chromatic")
    
    if len(sys.argv) > 1:
        binary_path = sys.argv[1]
    
    if not os.path.exists(binary_path):
        print(f"Binary not found: {binary_path}")
        print("Build first with: ./build.sh")
        return 1
    
    solve(binary_path)
    return 0

if __name__ == "__main__":
    sys.exit(main())
