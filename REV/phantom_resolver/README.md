# Phantom Resolver Challenge -- Building Documentation

This is just me documenting the building of Phantom Resolver Challenge... 

XZ-style IFUNC backdoor challenge for CTF competition.

## Structure

```
phantom_resolver/
├── src/                    # Source code (NOT distributed)
│   ├── libmonitor.c       # IFUNC library with backdoor
│   └── server_daemon.c    # Target binary
├── dist/                   # Files for contestants
│   ├── libmonitor.so      # Stripped library
│   ├── server_daemon.template  # Needs patching
│   └── README.md          # Challenge description
├── solution/               # Reference solution
│   └── solve.py           #  solver
└── Makefile              # Build automation
```

## Building

1. Compile `libmonitor.so` and strip symbols
2. Build `server_daemon.template`
3. Calculate and display the binary hash
4. Patch magic bytes into the template

## Solve.py

The solver will:
1. Extract constants from the library
2. Patch the template binary
3. Run it with correct arguments
4. Capture the flag

Upload `phantom_resolver_challenge.zip` containing:
- `libmonitor.so`
- `server_daemon.template`
- `README.md`

## Challenge Mechanics

### Checks Performed by IFUNC Resolver:

1. **Binary Hash Check**: Hashes first 4KB of calling process
2. **Magic Bytes Check**: Looks for "CHROMA" at offset 0x1000
3. **Command Line Check**: Requires --daemon flag

All three must pass to trigger backdoor.

### Red Herrings Included:

- Fake AES constants and decryption function
- Decoy environment variable checks
- Fake anti-debug function
- Fake backdoor function
- Misleading exported functions

## **Flag format:** `L3m0nCTF{...}`

