# CTF Challenges Discussion Summary

This document summarizes all the CTF challenges discussed in this conversation session.

---

## 1. Phantom Resolver (Completed ✅)

**Category:** Binary Exploitation / Reverse Engineering  
**Difficulty:** Hard to Insane  
**Flag:** `L3m0nCTF{ph4nt0m_r3s0lv3r_1func_m4st3ry}`

### Concept
XZ-style backdoor using IFUNC (Indirect Function) resolution. The challenge simulates a malicious shared library that activates a backdoor only when specific conditions are met.

### Key Features
- IFUNC resolver with multiple validation layers
- Binary hash/magic byte verification
- Command-line argument checks (`--daemon` flag)
- XOR-encoded flag with **dynamic key derivation**
- Red herrings: fake AES constants, decoy functions, fake backdoors

### Dynamic Key Generation
- Key derived from CHROMA magic bytes at offset 0x3000
- Calculation: `'C'^'H'^'R'^'O'^'M'^'A' = 0x1A`
- Flag XOR-encoded with this dynamic key
- Prevents simple `strings` command from revealing flag

### Files Built
- `libmonitor.so` - Malicious shared library
- `server_daemon` - Target binary that loads the library
- Solution script in Python

---

## 2. Ouroboros Archive (Built, Docker Testing In Progress ⚠️)

**Category:** Binary Exploitation  
**Difficulty:** Hard  
**CVE Reference:** CVE-2018-1002200 (Zip Slip)  
**Flag:** `L3m0nCTF{symb0l1c_l1nk_3sc4p3_m4st3r}`

### Concept
Vulnerable ZIP extraction utility with flawed path sanitizer. Contestants must craft a malicious ZIP containing symlinks to escape the sandbox.

### The Vulnerability
```c
// Only checks string PREFIX - doesn't resolve symlinks!
if (strncmp(path, "sandbox/", 8) != 0) {
    return 0;  // Blocked
}
return 1;  // BUG: Symlinks not resolved!
```

### Exploit Flow
1. Create ZIP with symlink: `escape` → `/`
2. Add file: `escape/tmp/pwned`
3. Sanitizer sees: `sandbox/escape/tmp/pwned` ✓
4. Extraction follows symlink → writes to `/tmp/pwned`

### Security Features
- Flag XOR-encoded with dynamic key (0x4c)
- Key derived from "OUROBOROS_KEY" magic at offset 0x4000
- Exploit verification: checks symlink exists + file written outside sandbox
- Flag NOT visible in `strings` output

### Docker Deployment
- Netcat server on port 1337
- Uses libzip for ZIP parsing
- SUID binary setup
- Resource limits (256MB RAM)

### Current Status
- Binary works locally ✅
- Docker container builds ✅
- Testing netcat communication (needs libzip dependency fix)

---

## 3. Graph Coloring Challenge (Plan Created 📋)

**Category:** Algorithmic / NP-Complete  
**Difficulty:** Hard  
**Flag:** `L3m0nCTF{gr4ph_c0l0r1ng_np_h4rd_but_n0t_1mp0ss1bl3}`

### Concept (Initial Draft)
Contestants receive a graph and must find a valid k-coloring (no adjacent vertices share color). Graph has hidden structure making it solvable, but brute force times out.

### Proposed Features
- Netcat server generates unique graph per connection
- Time limit (5 seconds) prevents brute force
- Graph has disguised bipartite-like structure
- DSatur algorithm as intended solution

### Questions Raised
- 5 second time limit - is it enough for users?
- Binary vs netcat delivery - which is more secure?

---

## 4. Chromatic Trilogy (Awaiting Details ❓)

User mentioned this challenge but no details were discussed in this session. Clarification needed on:
- What is the core concept?
- What category (stego, crypto, reverse)?
- What format (binary, web, file)?

---

## Technical Patterns Established

### Flag Encoding Pattern
All challenges use:
1. XOR encoding with dynamic key
2. Key derived from binary content (magic bytes)
3. `strings` command cannot reveal flag
4. Requires binary analysis to decode

### Directory Structure Pattern
```
challenge_name/
├── src/           # Source code
├── dist/          # Distribution files
├── solution/      # Reference solution
├── docker/        # Docker deployment
├── build.sh       # Build script
└── README.md      # Developer docs
```

### Docker Pattern
- Netcat-based delivery
- Resource limits
- Timeout protection
- SUID setup where needed

---

## Challenge Ranking (From Earlier Discussion)

Based on earlier ranking discussions:
1. Phantom Resolver - IFUNC backdoor ✅
2. Ouroboros Archive - Zip Slip ⚠️
3. Graph Coloring - NP-Complete 📋
4. Chromatic Trilogy - TBD ❓

---

## Next Steps

1. **Ouroboros Archive:** Fix Docker libzip dependency, complete testing
2. **Chromatic Trilogy:** Get requirements from user
3. **Graph Coloring:** Finalize design decisions (time limit, delivery method)
4. **GCP Deployment:** Deploy completed challenges to Google Cloud
