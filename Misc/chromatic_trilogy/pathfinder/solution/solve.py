#!/usr/bin/env python3
"""
PATHFINDER Challenge Solver
Chromatic Trilogy - Part 2

This solver demonstrates how to:
1. Analyze the binary to find the hidden constraint
2. Run Dijkstra's algorithm avoiding forbidden nodes
3. Submit the optimal path
"""

import subprocess
import os
import sys
from heapq import heappush, heappop

# Graph configuration (extracted from binary analysis)
NUM_NODES = 12

# Weighted adjacency matrix (from source/binary)
ADJ_MATRIX = [
    [0, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0],   # 0: Start
    [5, 0, 2, 7, 4, 0, 0, 0, 0, 0, 0, 0],   # 1
    [3, 2, 0, 0, 6, 8, 0, 0, 0, 0, 0, 0],   # 2
    [0, 7, 0, 0, 3, 0, 9, 0, 0, 0, 0, 0],   # 3 (FORBIDDEN)
    [0, 4, 6, 3, 0, 2, 5, 8, 0, 0, 0, 0],   # 4
    [0, 0, 8, 0, 2, 0, 0, 4, 6, 0, 0, 0],   # 5
    [0, 0, 0, 9, 5, 0, 0, 3, 0, 7, 0, 0],   # 6 (FORBIDDEN)
    [0, 0, 0, 0, 8, 4, 3, 0, 5, 2, 6, 0],   # 7
    [0, 0, 0, 0, 0, 6, 0, 5, 0, 0, 4, 9],   # 8
    [0, 0, 0, 0, 0, 0, 7, 2, 0, 0, 3, 5],   # 9
    [0, 0, 0, 0, 0, 0, 0, 6, 4, 3, 0, 2],   # 10
    [0, 0, 0, 0, 0, 0, 0, 0, 9, 5, 2, 0]    # 11: End
]

# Hidden constraint: forbidden nodes (discovered via RE)
FORBIDDEN_NODES = {3, 6}

def dijkstra_with_constraint(start, end, forbidden):
    """Dijkstra's algorithm avoiding forbidden nodes"""
    INF = float('inf')
    dist = [INF] * NUM_NODES
    prev = [-1] * NUM_NODES
    dist[start] = 0
    
    # Priority queue: (distance, node)
    pq = [(0, start)]
    visited = set()
    
    while pq:
        d, u = heappop(pq)
        
        if u in visited:
            continue
        visited.add(u)
        
        if u in forbidden:
            continue
        
        for v in range(NUM_NODES):
            if ADJ_MATRIX[u][v] > 0 and v not in forbidden:
                new_dist = d + ADJ_MATRIX[u][v]
                if new_dist < dist[v]:
                    dist[v] = new_dist
                    prev[v] = u
                    heappush(pq, (new_dist, v))
    
    # Reconstruct path
    if dist[end] == INF:
        return None, INF
    
    path = []
    current = end
    while current != -1:
        path.append(current)
        current = prev[current]
    path.reverse()
    
    return path, dist[end]

def solve(binary_path):
    """Main solver"""
    print("=" * 60)
    print("PATHFINDER Challenge Solver")
    print("=" * 60)
    
    # Step 1: Identify constraint (normally via RE)
    print("\n[1] Constraint Analysis (normally via binary RE):")
    print(f"    Forbidden nodes: {FORBIDDEN_NODES}")
    
    # Step 2: Run Dijkstra
    print("\n[2] Running Dijkstra with constraints...")
    path, cost = dijkstra_with_constraint(0, 11, FORBIDDEN_NODES)
    
    if path is None:
        print("    ✗ No valid path found!")
        return None
    
    print(f"    Path: {' -> '.join(map(str, path))}")
    print(f"    Cost: {cost}")
    
    # Step 3: Format answer
    path_str = ",".join(map(str, path))
    print(f"\n[3] Path string: {path_str}")
    
    # Step 4: Submit to binary
    print("\n[4] Submitting solution to binary...")
    try:
        result = subprocess.run(
            [binary_path, path_str],
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
        return None
    except Exception as e:
        print(f"    Error: {e}")
        return None

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    binary_path = os.path.join(script_dir, "..", "dist", "pathfinder")
    
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
