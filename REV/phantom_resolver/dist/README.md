# Phantom Resolver

**Category:** Reverse Engineering  
**Difficulty:** Hard  
**Points:** 500

## Description

Our security team deployed a new monitoring library (`libmonitor.so`) across all servers. The vendor claims it provides "enhanced runtime security validation."

Everything seems normal, but something feels... off.

Your task: Figure out what this library is really doing.

## Files Provided

- `libmonitor.so` - The monitoring library (stripped binary)
- `server_daemon.template` - Server binary that uses the library

## Hints

1. The library's behavior changes based on the process loading it
2. Some binaries are more equal than others
3. IFUNC resolvers execute before `main()` 
4. The real CVE-2024-3094 (XZ backdoor) used similar techniques

## Goal

Make the system reveal its secrets.

## Environment

- Linux x86_64
- glibc 2.31+
- Standard debugging tools available

## Notes

The template binary may need... adjustments.

---

**Flag format:** `flag{...}`
