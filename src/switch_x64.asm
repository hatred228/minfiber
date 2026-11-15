; file: switch_x64.asm
; target: Windows x64 ABI (MASM / ml64)
; description: Low-level cooperative context switch and entry trampoline.

.code

; extern "C" void minfiber_switch(void** from_rsp, void* to_rsp);
; rcx = from_rsp (pointer to current fiber's saved RSP)
; rdx = to_rsp   (target fiber's saved RSP)
minfiber_switch PROC
    ; 1. Push general-purpose callee-saved registers (8 * 8 = 64 bytes)
    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15

    ; 2. Push RFLAGS (8 bytes)
    pushfq

    ; 3. Save TEB stack boundaries (StackBase at GS:[08h], StackLimit at GS:[10h])
    mov rax, gs:[08h]
    push rax
    mov rax, gs:[10h]
    push rax

    ; 4. Allocate 160 bytes for callee-saved XMM registers (XMM6 - XMM15, 10 * 16 = 160 bytes)
    sub rsp, 160
    movdqu [rsp + 0*16], xmm6
    movdqu [rsp + 1*16], xmm7
    movdqu [rsp + 2*16], xmm8
    movdqu [rsp + 3*16], xmm9
    movdqu [rsp + 4*16], xmm10
    movdqu [rsp + 5*16], xmm11
    movdqu [rsp + 6*16], xmm12
    movdqu [rsp + 7*16], xmm13
    movdqu [rsp + 8*16], xmm14
    movdqu [rsp + 9*16], xmm15

    ; 5. Save current RSP to *from_rsp
    mov [rcx], rsp

    ; 6. Switch to new RSP
    mov rsp, rdx

    ; 7. Restore XMM registers
    movdqu xmm6,  [rsp + 0*16]
    movdqu xmm7,  [rsp + 1*16]
    movdqu xmm8,  [rsp + 2*16]
    movdqu xmm9,  [rsp + 3*16]
    movdqu xmm10, [rsp + 4*16]
    movdqu xmm11, [rsp + 5*16]
    movdqu xmm12, [rsp + 6*16]
    movdqu xmm13, [rsp + 7*16]
    movdqu xmm14, [rsp + 8*16]
    movdqu xmm15, [rsp + 9*16]
    add rsp, 160

    ; 8. Restore TEB stack boundaries
    pop rax
    mov gs:[10h], rax
    pop rax
    mov gs:[08h], rax

    ; 9. Restore RFLAGS
    popfq

    ; 10. Restore general-purpose callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx

    ; 11. Return into resumed fiber's caller / return address on top of stack
    ret
minfiber_switch ENDP

; extern "C" void minfiber_trampoline(void);
minfiber_trampoline PROC
    sub rsp, 32

    mov rcx, r13
    call r12

    add rsp, 32

    extern minfiber_on_exit : PROC
    sub rsp, 32
    call minfiber_on_exit
    add rsp, 32

trampoline_halt:
    pause
    jmp trampoline_halt
minfiber_trampoline ENDP

END
