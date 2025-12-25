/* ============================================
   VAULT BREACH - ENHANCED SCRIPT
   Anti-debugging, Matrix Rain, Terminal Effects
   ============================================ */

// ==========================================
// ANTI-DEBUGGING & SECURITY MEASURES
// ==========================================

// Obfuscated helper name to make reverse engineering harder
const _0x4f72 = (n) => { let r=0; while(n!==0){r^=n&1;n>>>=1;} return r; };

// Timing-based debugger detection
let _lastCheck = Date.now();
let _debugWarnings = 0;

function _checkTiming() {
    const now = Date.now();
    const diff = now - _lastCheck;
    _lastCheck = now;
    
    // If execution took too long, debugger might be attached
    if (diff > 200) {
        _debugWarnings++;
        if (_debugWarnings > 3) {
            addTerminalOutput("[!] ANOMALY DETECTED: Timing irregularities", "warning");
            // Corrupt state slightly - make challenge harder
            _corruptionLevel++;
        }
    }
}

// DevTools detection via console timing
let _devToolsOpen = false;
const _detectDevTools = () => {
    const threshold = 160;
    const start = performance.now();
    debugger; // This line causes pause if devtools open
    const end = performance.now();
    
    if (end - start > threshold) {
        if (!_devToolsOpen) {
            _devToolsOpen = true;
            addTerminalOutput("[!] INTRUSION DETECTION: Analysis tools detected", "warning");
            addTerminalOutput("[!] Security protocols engaged...", "error");
        }
    }
};

// Disabled console methods in production (uncomment for harder challenge)
// const _noop = () => {};
// ['log', 'debug', 'info', 'warn', 'error', 'table', 'trace'].forEach(m => {
//     console[m] = _noop;
// });

let _corruptionLevel = 0;
let attemptCount = 0;
let isProcessing = false;
let cooldownActive = false;
let wasmModule = null;

// ==========================================
// MATRIX RAIN BACKGROUND
// ==========================================

function initMatrixRain() {
    const canvas = document.getElementById('matrix-bg');
    const ctx = canvas.getContext('2d');
    
    // Set canvas size
    function resize() {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    resize();
    window.addEventListener('resize', resize);
    
    // Matrix characters (mix of katakana, numbers, and symbols)
    const chars = 'アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲン0123456789ABCDEF@#$%^&*(){}[]|;:<>?';
    const charArray = chars.split('');
    
    const fontSize = 14;
    const columns = Math.floor(canvas.width / fontSize);
    
    // Array to track y position of each column
    const drops = [];
    for (let i = 0; i < columns; i++) {
        drops[i] = Math.random() * -100;
    }
    
    function draw() {
        // Semi-transparent black to create fade effect
        ctx.fillStyle = 'rgba(10, 10, 10, 0.05)';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        ctx.fillStyle = '#00ff41';
        ctx.font = `${fontSize}px monospace`;
        
        for (let i = 0; i < drops.length; i++) {
            // Random character
            const char = charArray[Math.floor(Math.random() * charArray.length)];
            
            // Draw character
            ctx.fillText(char, i * fontSize, drops[i] * fontSize);
            
            // Reset drop randomly after reaching bottom
            if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
                drops[i] = 0;
            }
            
            drops[i]++;
        }
    }
    
    // Run animation
    setInterval(draw, 50);
}

// ==========================================
// TERMINAL OUTPUT SIMULATION
// ==========================================

const bootSequence = [
    { text: "[SYSTEM] Initializing vault breach protocol...", type: "info", delay: 0 },
    { text: "[SYSTEM] Loading cryptographic modules...", type: "info", delay: 400 },
    { text: "[WASM] WebAssembly binary loaded (5.6KB)", type: "success", delay: 800 },
    { text: "[SECURITY] AES-256-GCM encryption detected", type: "warning", delay: 1200 },
    { text: "[SYSTEM] Ready for access key input", type: "success", delay: 1600 },
];

function runBootSequence() {
    const output = document.getElementById('terminal-output');
    output.innerHTML = '';
    
    bootSequence.forEach((item, index) => {
        setTimeout(() => {
            addTerminalOutput(item.text, item.type);
        }, item.delay);
    });
}

function addTerminalOutput(text, type = 'info') {
    const output = document.getElementById('terminal-output');
    const line = document.createElement('div');
    line.className = `output-line ${type}`;
    line.textContent = text;
    output.appendChild(line);
    output.scrollTop = output.scrollHeight;
    
    // Limit lines to prevent overflow
    while (output.children.length > 15) {
        output.removeChild(output.firstChild);
    }
}

// ==========================================
// WASM LOADING
// ==========================================

async function loadWasm() {
    try {
        wasmModule = await WebAssembly.instantiateStreaming(fetch("vault.wasm"), {
            env: {
                // Renamed from 'x' to make it less obvious
                x: _0x4f72
            }
        });
        
        document.getElementById('wasm-status').textContent = 'READY';
        document.getElementById('wasm-status').style.color = '#00ff41';
        
        return true;
    } catch (error) {
        document.getElementById('wasm-status').textContent = 'ERROR';
        document.getElementById('wasm-status').style.color = '#ff0040';
        addTerminalOutput("[ERROR] Failed to load WASM module: " + error.message, "error");
        return false;
    }
}

// ==========================================
// FAKE DECRYPTION PROGRESS
// ==========================================

function showProgress(callback) {
    const container = document.getElementById('progress-container');
    const fill = document.getElementById('progress-fill');
    const percentage = document.getElementById('progress-percentage');
    
    container.classList.add('active');
    
    let progress = 0;
    const interval = setInterval(() => {
        // Random increment for realistic feel
        progress += Math.random() * 15 + 5;
        if (progress > 100) progress = 100;
        
        fill.style.width = `${progress}%`;
        percentage.textContent = `${Math.floor(progress)}%`;
        
        // Add fake processing messages
        if (progress > 20 && progress < 25) {
            addTerminalOutput("[DECRYPT] Analyzing key structure...", "info");
        } else if (progress > 50 && progress < 55) {
            addTerminalOutput("[DECRYPT] Verifying hash integrity...", "info");
        } else if (progress > 80 && progress < 85) {
            addTerminalOutput("[DECRYPT] Final validation...", "info");
        }
        
        if (progress >= 100) {
            clearInterval(interval);
            setTimeout(() => {
                container.classList.remove('active');
                fill.style.width = '0%';
                percentage.textContent = '0%';
                callback();
            }, 300);
        }
    }, 100);
}

// ==========================================
// MAIN UNLOCK FUNCTION
// ==========================================

function executeUnlock() {
    // Anti-debugging check
    _checkTiming();
    
    if (isProcessing || cooldownActive) return;
    
    const input = document.getElementById('vault-input');
    const flag = input.value.trim();
    
    // Validate input
    if (!flag) {
        addTerminalOutput("[ERROR] Access key cannot be empty", "error");
        shakeElement(input.parentElement);
        return;
    }
    
    // Check format hint
    if (!flag.startsWith('L3m0nCTF{') || !flag.endsWith('}')) {
        addTerminalOutput("[WARN] Invalid key format detected", "warning");
    }
    
    isProcessing = true;
    attemptCount++;
    document.getElementById('attempt-count').textContent = attemptCount;
    
    // Update button state
    const btn = document.getElementById('execute-btn');
    btn.classList.add('loading');
    btn.disabled = true;
    
    addTerminalOutput(`[BREACH] Attempting unlock with key: ${flag.substring(0, 10)}...`, "info");
    
    // Show fake progress
    showProgress(() => {
        performUnlock(flag);
    });
}

function performUnlock(flag) {
    if (!wasmModule) {
        addTerminalOutput("[ERROR] WASM module not loaded", "error");
        resetUnlockState();
        return;
    }
    
    try {
        const flagEncoded = new TextEncoder().encode(flag);
        
        if (flagEncoded.length >= 0x100) {
            addTerminalOutput("[ERROR] Key exceeds maximum length", "error");
            handleFailure();
            return;
        }
        
        // Apply corruption if debugging was detected
        let modifiedFlag = flagEncoded;
        if (_corruptionLevel > 2) {
            addTerminalOutput("[!] Memory corruption detected in key buffer", "error");
            // Silently corrupt the input (makes challenge harder if debugging)
            modifiedFlag = new Uint8Array(flagEncoded);
            modifiedFlag[0] ^= _corruptionLevel;
        }
        
        // Write to WASM memory
        new Uint8Array(wasmModule.instance.exports.memory.buffer).set(modifiedFlag);
        new Uint8Array(wasmModule.instance.exports.memory.buffer).set([0], modifiedFlag.length);
        
        // Call unlock function
        const result = wasmModule.instance.exports.unlock() !== 0;
        
        if (result) {
            handleSuccess(flag);
        } else {
            handleFailure();
        }
    } catch (error) {
        addTerminalOutput("[ERROR] " + error.message, "error");
        handleFailure();
    }
}

function handleSuccess(flag) {
    addTerminalOutput("[SUCCESS] ACCESS GRANTED!", "success");
    addTerminalOutput("[SUCCESS] Vault decryption complete", "success");
    
    // Update lock visual
    const lockIcon = document.getElementById('lock-icon');
    const lockShackle = document.getElementById('lock-shackle');
    const lockStatus = document.getElementById('lock-status');
    const statusDot = document.querySelector('.status-dot');
    const statusText = document.getElementById('status-text');
    
    lockShackle.classList.add('unlocked');
    lockStatus.textContent = '🔓 UNLOCKED';
    lockStatus.classList.add('unlocked');
    statusDot.classList.add('connected');
    statusText.textContent = 'DECRYPTED';
    statusText.style.color = '#00ff41';
    
    // Show success overlay
    setTimeout(() => {
        const overlay = document.getElementById('success-overlay');
        const flagDisplay = document.getElementById('success-flag');
        flagDisplay.textContent = flag;
        overlay.classList.add('active');
    }, 1000);
    
    resetUnlockState();
}

function handleFailure() {
    addTerminalOutput("[FAILED] ACCESS DENIED - Invalid key", "error");
    
    // Shake effects
    const lockIcon = document.getElementById('lock-icon');
    const input = document.getElementById('vault-input');
    
    lockIcon.classList.add('error');
    shakeElement(document.querySelector('.terminal-container'));
    
    setTimeout(() => {
        lockIcon.classList.remove('error');
    }, 500);
    
    // Start cooldown
    startCooldown();
    resetUnlockState();
}

function resetUnlockState() {
    isProcessing = false;
    const btn = document.getElementById('execute-btn');
    btn.classList.remove('loading');
    if (!cooldownActive) {
        btn.disabled = false;
    }
}

function startCooldown() {
    cooldownActive = true;
    const btn = document.getElementById('execute-btn');
    const timer = document.getElementById('cooldown-timer');
    btn.disabled = true;
    
    let seconds = 3 + Math.floor(attemptCount / 3); // Increase cooldown with attempts
    if (seconds > 10) seconds = 10;
    
    timer.textContent = `(${seconds}s cooldown)`;
    
    const interval = setInterval(() => {
        seconds--;
        timer.textContent = `(${seconds}s cooldown)`;
        
        if (seconds <= 0) {
            clearInterval(interval);
            timer.textContent = '';
            cooldownActive = false;
            btn.disabled = false;
        }
    }, 1000);
}

function shakeElement(element) {
    element.classList.add('shake');
    setTimeout(() => {
        element.classList.remove('shake');
    }, 500);
}

// ==========================================
// EVENT LISTENERS & INITIALIZATION
// ==========================================

document.addEventListener('DOMContentLoaded', () => {
    // Initialize matrix rain
    initMatrixRain();
    
    // Run boot sequence
    setTimeout(runBootSequence, 500);
    
    // Load WASM
    loadWasm();
    
    // Update memory display periodically
    setInterval(() => {
        const mem = document.getElementById('mem-usage');
        const usage = 64 + Math.floor(Math.random() * 32);
        mem.textContent = `${usage}KB`;
    }, 2000);
    
    // Enter key to submit
    document.getElementById('vault-input').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            executeUnlock();
        }
    });
    
    // Periodic anti-debugging check (disabled by default - uncomment for harder challenge)
    // setInterval(_detectDevTools, 1000);
});

// Prevent right-click (optional - can be annoying)
// document.addEventListener('contextmenu', e => e.preventDefault());

// Console warning message
console.log('%c⚠️ WARNING', 'color: #ff0040; font-size: 24px; font-weight: bold;');
console.log('%cThis is a CTF challenge. Debugging is expected, but beware of anti-tampering measures.', 'color: #00ff41; font-size: 14px;');
console.log('%cFlag format: L3m0nCTF{...}', 'color: #00ffff; font-size: 12px;');
