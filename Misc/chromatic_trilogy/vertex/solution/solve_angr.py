import angr
import sys

def solve():
    # Load the binary
    p = angr.Project('../dist/vertex', auto_load_libs=False)

    # Create the entry state
    state = p.factory.entry_state()
    simgr = p.factory.simulation_manager(state)

    # Define success/fail conditions based on your dump
    # Success: "CHALLENGE COMPLETE" [cite: 2]
    # Failure: "Incorrect." [cite: 1]
    
    def is_success(state):
        stdout = state.posix.dumps(sys.stdout.fileno())
        return b"CHALLENGE COMPLETE" in stdout

    def is_failure(state):
        stdout = state.posix.dumps(sys.stdout.fileno())
        return b"Incorrect" in stdout

    print("[-] Exploring with Angr...")
    
    # Enable aggressive exploration
    simgr.explore(find=is_success, avoid=is_failure)

    if simgr.found:
        solution_state = simgr.found[0]
        solution = solution_state.posix.dumps(sys.stdin.fileno())
        print(f"[+] SOLUTION FOUND: {solution.decode().strip()}")
        
        # If the solution is binary data, print hex as well
        # print(f"[+] Hex: {solution.hex()}")
    else:
        print("[!] No solution found. The state might be pruning too early.")

if __name__ == "__main__":
    solve()
