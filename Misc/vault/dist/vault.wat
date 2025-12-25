;; VAULT.WAT - CTF Challenge with Opaque Predicates
;; Flag: L3m0nCTF{w4sm_0p4qu3_pr3d1c4t3s_4r3_fun}
;; Compile: wat2wasm vault.wat -o vault.wasm

(module
  ;; Import parity function from JavaScript (the opaque predicate oracle)
  (import "env" "x" (func $parity (param i32) (result i32)))
  
  ;; Memory: 1 page = 64KB
  ;; Layout:
  ;;   0-255: Input buffer
  ;;   512-1023: Scratch space
  ;;   3584-3700: Encrypted flag data
  (memory (export "memory") 1)
  
  ;; Encrypted flag data at offset 3584
  ;; Flag "L3m0nCTF{w4sm_0p4qu3_pr3d1c4t3s_4r3_fun}" XOR'd with key
  (data (i32.const 3584) 
    "\16\0F\12\2E\F3\08\7E\CA\15\7A\C7\D4\34\9E\0B\F2\6E\4D\0A\2D\C2\3B\58\BF\0A\3C\90\93\2D\F2\48\DD\6E\4E\4C\41\FB\3E\44\F1")
  
  ;; XOR key at offset 3700
  (data (i32.const 3700)
    "\5A\3C\7F\1E\9D\4B\2A\8C\6E\0D\F3\A7\59\C1\3B\82")
  
  ;; Prefix "L3m0nCTF{" at offset 3720
  (data (i32.const 3720)
    "\4C\33\6D\30\6E\43\54\46\7B")
  
  ;; Constants for opaque predicates (these have known parity values)
  ;; 0x100 = 256 -> binary has 1 one -> parity = 1 (true)
  ;; 0x103 = 259 -> binary has 3 ones -> parity = 1 (true)
  ;; 0x165 = 357 -> binary has 5 ones -> parity = 1 (true)
  ;; 0x101 = 257 -> binary has 2 ones -> parity = 0 (false)
  ;; 0x102 = 258 -> binary has 2 ones -> parity = 0 (false)
  
  ;; Get string length
  (func $strlen (param $ptr i32) (result i32)
    (local $len i32)
    (local.set $len (i32.const 0))
    (block $break
      (loop $loop
        ;; Check if current byte is 0
        (br_if $break
          (i32.eqz (i32.load8_u (i32.add (local.get $ptr) (local.get $len)))))
        ;; Check if length > 255
        (br_if $break
          (i32.gt_u (local.get $len) (i32.const 255)))
        ;; Increment length
        (local.set $len (i32.add (local.get $len) (i32.const 1)))
        (br $loop)
      )
    )
    (local.get $len)
  )
  
  ;; Fake transformation (decoy - never used but appears in binary)
  (func $fake_transform (param $ptr i32) (param $len i32)
    (local $i i32)
    (local.set $i (i32.const 0))
    (block $break
      (loop $loop
        (br_if $break (i32.ge_u (local.get $i) (local.get $len)))
        (i32.store8 
          (i32.add (local.get $ptr) (local.get $i))
          (i32.xor 
            (i32.load8_u (i32.add (local.get $ptr) (local.get $i)))
            (i32.const 0xDE)))
        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br $loop)
      )
    )
  )
  
  ;; Another fake function
  (func $fake_hash (param $ptr i32) (param $len i32) (result i32)
    (i32.xor 
      (i32.load (local.get $ptr))
      (i32.const 0xDEADBEEF))
  )
  
  ;; Hash function for additional verification
  (func $mix_hash (param $ptr i32) (param $len i32) (result i32)
    (local $hash i32)
    (local $i i32)
    (local $byte i32)
    (local $shift i32)
    (local.set $hash (i32.const 0x1337BEEF))
    (local.set $i (i32.const 0))
    (block $break
      (loop $loop
        (br_if $break (i32.ge_u (local.get $i) (local.get $len)))
        
        ;; Opaque predicate for confusion
        (if (call $parity (i32.const 0x100))
          (then
            ;; Real hash computation
            (local.set $byte (i32.load8_u (i32.add (local.get $ptr) (local.get $i))))
            (local.set $shift (i32.rem_u (local.get $i) (i32.const 24)))
            (local.set $hash
              (i32.xor 
                (local.get $hash)
                (i32.shl (local.get $byte) (local.get $shift))))
            (local.set $hash
              (i32.or
                (i32.shl (local.get $hash) (i32.const 5))
                (i32.shr_u (local.get $hash) (i32.const 27))))
          )
        )
        
        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br $loop)
      )
    )
    (local.get $hash)
  )
  
  ;; Verify prefix "L3m0nCTF{"
  (func $check_prefix (param $input i32) (result i32)
    (local $i i32)
    (local $prefix_ptr i32)
    (local.set $prefix_ptr (i32.const 3720))
    (local.set $i (i32.const 0))
    
    ;; Opaque predicate - always true path
    (if (call $parity (i32.const 0x101))
      (then
        ;; Dead code path
        (return (i32.const 1))
      )
    )
    
    (block $fail
      (loop $loop
        (br_if $fail
          (i32.ne
            (i32.load8_u (i32.add (local.get $input) (local.get $i)))
            (i32.load8_u (i32.add (local.get $prefix_ptr) (local.get $i)))))
        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br_if $loop (i32.lt_u (local.get $i) (i32.const 9)))
        (return (i32.const 1))
      )
    )
    (i32.const 0)
  )
  
  ;; Verify suffix "}"
  (func $check_suffix (param $input i32) (param $len i32) (result i32)
    ;; Opaque predicate
    (if (result i32) (call $parity (i32.const 0x100))
      (then
        ;; Real check
        (i32.eq
          (i32.load8_u (i32.add (local.get $input) (i32.sub (local.get $len) (i32.const 1))))
          (i32.const 0x7D))  ;; '}'
      )
      (else
        ;; Fake check - never runs
        (i32.eq (local.get $len) (i32.const 999))
      )
    )
  )
  
  ;; XOR transform and compare with encrypted flag
  (func $verify_content (param $input i32) (param $len i32) (result i32)
    (local $i i32)
    (local $input_byte i32)
    (local $key_byte i32)
    (local $enc_byte i32)
    (local $transformed i32)
    
    (local.set $i (i32.const 0))
    
    (block $success
      (block $fail
        (loop $loop
          ;; If we've checked all bytes, break to success
          (br_if $success (i32.ge_u (local.get $i) (local.get $len)))
          
          ;; Opaque predicate for obfuscation
          (if (call $parity (i32.const 0x103))
            (then
              ;; Real verification path
              (local.set $input_byte 
                (i32.load8_u (i32.add (local.get $input) (local.get $i))))
              
              ;; Get XOR key byte (key is at 3700, 16 bytes)
              (local.set $key_byte
                (i32.load8_u 
                  (i32.add 
                    (i32.const 3700) 
                    (i32.rem_u (local.get $i) (i32.const 16)))))
              
              ;; XOR input with key
              (local.set $transformed
                (i32.xor (local.get $input_byte) (local.get $key_byte)))
              
              ;; Get expected encrypted byte
              (local.set $enc_byte
                (i32.load8_u (i32.add (i32.const 3584) (local.get $i))))
              
              ;; Compare
              (if (i32.ne (local.get $transformed) (local.get $enc_byte))
                (then
                  ;; More obfuscation - fake path
                  (if (call $parity (i32.const 0x101))
                    (then
                      (call $fake_transform (local.get $input) (local.get $len))
                    )
                  )
                  (br $fail)
                )
              )
            )
            (else
              ;; Dead code
              (call $fake_transform (local.get $input) (local.get $len))
            )
          )
          
          (local.set $i (i32.add (local.get $i) (i32.const 1)))
          (br $loop)
        )
      )
      ;; Fail path - return 0
      (return (i32.const 0))
    )
    ;; Success path - return 1
    (i32.const 1)
  )
  
  ;; Main unlock function - EXPORTED
  (func (export "unlock") (result i32)
    (local $input i32)
    (local $len i32)
    (local $hash i32)
    
    ;; Input is at memory offset 0
    (local.set $input (i32.const 0))
    
    ;; Get input length
    (local.set $len (call $strlen (local.get $input)))
    
    ;; Stage 1: Length check (flag is 40 chars)
    (if (call $parity (i32.const 0x100))
      (then
        ;; Real length check
        (if (i32.ne (local.get $len) (i32.const 40))
          (then
            ;; Fake path for confusion
            (if (call $parity (i32.const 0x101))
              (then
                (drop (call $fake_hash (local.get $input) (local.get $len)))
              )
            )
            (return (i32.const 0))
          )
        )
      )
      (else
        ;; Dead code
        (if (i32.lt_u (local.get $len) (i32.const 5))
          (then (return (i32.const 0)))
        )
      )
    )
    
    ;; Stage 2: Prefix check
    (if (call $parity (i32.const 0x103))
      (then
        (if (i32.eqz (call $check_prefix (local.get $input)))
          (then (return (i32.const 0)))
        )
      )
      (else
        ;; Dead code path
        (return (call $fake_hash (local.get $input) (local.get $len)))
      )
    )
    
    ;; Stage 3: Suffix check
    (if (call $parity (i32.const 0x165))
      (then
        (if (i32.eqz (call $check_suffix (local.get $input) (local.get $len)))
          (then (return (i32.const 0)))
        )
      )
    )
    
    ;; Stage 4: Full content verification
    (if (call $parity (i32.const 0x100))
      (then
        (if (i32.eqz (call $verify_content (local.get $input) (local.get $len)))
          (then
            ;; More confusion
            (if (call $parity (i32.const 0x101))
              (then
                (return (i32.and 
                  (call $mix_hash (local.get $input) (local.get $len))
                  (i32.const 1)))
              )
            )
            (return (i32.const 0))
          )
        )
      )
    )
    
    ;; Stage 5: Hash check for extra difficulty
    (local.set $hash (call $mix_hash (local.get $input) (local.get $len)))
    (if (call $parity (i32.const 0x103))
      (then
        ;; Check lower 16 bits of hash
        (if (i32.ne 
              (i32.and (local.get $hash) (i32.const 0xFFFF))
              (i32.const 0xD9CE))
          (then (return (i32.const 0)))
        )
      )
    )
    
    ;; Success!
    (i32.const 1)
  )
)
