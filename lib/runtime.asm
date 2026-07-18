; Usage:
; RCX: Address (hint)
; RDX: Length
falloc:
    mov rax, 9          ; sys_mmap
    mov rdi, rcx        ; 1st arg: addr (from rcx)
    mov rsi, rdx        ; 2nd arg: len (from rdx)
    mov rdx, 3          ; 3rd arg: prot = PROT_READ (0x1) | PROT_WRITE (0x2)
    mov r10, 0x22       ; 4th arg: flags = MAP_PRIVATE (0x02) | MAP_ANONYMOUS (0x20)
    mov r8, -1          ; 5th arg: fd = -1
    mov r9, 0           ; 6th arg: offset = 0 (Crucial fix!)
    syscall             ; Note: This instruction will overwrite RCX and R11
    ret                 ; Fixed syntax

; Usage to grow memory:
; RAX: 25 (sys_mremap)
; RDI: Old address (the pointer returned by your first sys_mmap)
; RSI: Old size (e.g., 4096)
; RDX: New size (e.g., 4101)
; R10: 1 (MREMAP_MAYMOVE - allows the kernel to move the page if needed)
frealloc:
    mov rax, 25         ; sys_mremap
    mov rdi, rbx        ; Move your old memory pointer here
    mov rsi, 4096       ; Old size
    mov rdx, 4101       ; New size (Old size + 5 bytes)
    mov r10, 1          ; MREMAP_MAYMOVE flags
    syscall             ; RAX will contain the NEW address of the memory
    ret

; Usage:
; RCX: Address of allocated memory to free
; RDX: Length of the memory block
ffree:
    mov rax, 11         ; sys_munmap system call number
    mov rdi, rcx        ; 1st arg: starting address
    mov rsi, rdx        ; 2nd arg: length in bytes
    syscall             ; Invoke the kernel (overwrites RCX and R11)

    ; On success, RAX will be 0.
    ; On failure, RAX will contain a negative error code (e.g., -22 for EINVAL).
    ret