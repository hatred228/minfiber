# path: src/switch_x64.s
# context: GNU Assembler (MinGW-w64 x86_64)
# description: Cooperative fiber context switch and trampoline for Windows x64 ABI

.intel_syntax noprefix
.text

.globl minfiber_switch
.def minfiber_switch; .scl 2; .type 32; .endef
# void minfiber_switch(void** from_rsp, void* to_rsp)
# rcx = from_rsp, rdx = to_rsp
minfiber_switch:
    # 1. Save general-purpose callee-saved registers
    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15

    # 2. Save RFLAGS
    pushfq

    # 3. Save TEB stack boundaries (gs:[0x08] = StackBase, gs:[0x10] = StackLimit)
    mov rax, gs:[0x08]
    push rax
    mov rax, gs:[0x10]
    push rax

    # 4. Save callee-saved SIMD registers XMM6 - XMM15 (160 bytes)
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

    # 5. Save current RSP to *from_rsp
    mov [rcx], rsp

    # 6. Switch to new RSP
    mov rsp, rdx

    # 7. Restore XMM registers
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

    # 8. Restore TEB boundaries
    pop rax
    mov gs:[0x10], rax
    pop rax
    mov gs:[0x08], rax

    # 9. Restore RFLAGS
    popfq

    # 10. Restore GPRs
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx

    ret

.globl minfiber_trampoline
.def minfiber_trampoline; .scl 2; .type 32; .endef
# Trampoline entered upon fiber's first execution
# R12 = target function pointer: void (*)(void*)
# R13 = user argument: void*
minfiber_trampoline:
    # On entry, RSP % 16 == 0. Allocate 32 bytes shadow space.
    # RSP % 16 remains 0 before CALL.
    sub rsp, 32

    mov rcx, r13
    call r12

    add rsp, 32

    # Call exit handler
    sub rsp, 32
    call minfiber_on_exit
    add rsp, 32

trampoline_loop:
    pause
    jmp trampoline_loop
