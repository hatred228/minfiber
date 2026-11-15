// path: src/fiber.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/fiber.hpp"

#include <windows.h>
#include <system_error>
#include <iostream>
#include <cstring>

extern "C" {
    void minfiber_trampoline(void);
}

namespace minfiber {

namespace {

void fiber_entry_thunk(void* arg) {
    if (arg) {
        auto* fiber = static_cast<Fiber*>(arg);
        fiber->invoke();
    }
}

} // namespace

Fiber::Fiber(FiberFunction fn, std::size_t stack_size, std::string name)
    : fn_(std::move(fn)), name_(std::move(name)) {
    if (stack_size > 0) {
        allocate_stack(stack_size);
        init_stack_frame();
    }
}

Fiber::~Fiber() {
    free_stack();
}

Fiber::Fiber(Fiber&& other) noexcept
    : fn_(std::move(other.fn_)),
      name_(std::move(other.name_)),
      state_(other.state_),
      ctx_(other.ctx_) {
    other.ctx_ = {};
    other.state_ = FiberState::Terminated;
}

Fiber& Fiber::operator=(Fiber&& other) noexcept {
    if (this != &other) {
        free_stack();
        fn_ = std::move(other.fn_);
        name_ = std::move(other.name_);
        state_ = other.state_;
        ctx_ = other.ctx_;
        other.ctx_ = {};
        other.state_ = FiberState::Terminated;
    }
    return *this;
}

void Fiber::invoke() {
    if (fn_) {
        fn_();
    }
}

void Fiber::allocate_stack(std::size_t size) {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    const std::size_t page_size = sys_info.dwPageSize;

    // Align size to page boundary
    std::size_t aligned_size = (size + page_size - 1) & ~(page_size - 1);
    // Add 1 page for guard page
    std::size_t total_size = aligned_size + page_size;

    void* stack_memory = VirtualAlloc(
        nullptr,
        total_size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (!stack_memory) {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "VirtualAlloc failed for fiber stack"
        );
    }

    // Set guard page on the lowest page to catch stack overflows
    DWORD old_protect = 0;
    if (!VirtualProtect(stack_memory, page_size, PAGE_NOACCESS, &old_protect)) {
        VirtualFree(stack_memory, 0, MEM_RELEASE);
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "VirtualProtect failed to set guard page"
        );
    }

    ctx_.stack_limit = static_cast<char*>(stack_memory) + page_size;
    ctx_.stack_base = static_cast<char*>(stack_memory) + total_size;
    ctx_.stack_size = aligned_size;
    ctx_.rsp = ctx_.stack_base;
}

void Fiber::free_stack() {
    if (ctx_.stack_limit) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        void* raw_base = static_cast<char*>(ctx_.stack_limit) - sys_info.dwPageSize;
        VirtualFree(raw_base, 0, MEM_RELEASE);
        ctx_ = {};
    }
}

void Fiber::init_stack_frame() {
    // Stack layout expected by minfiber_switch when popping:
    //
    // [High memory / stack_base]
    // return_address      -> minfiber_trampoline
    // saved_rbx           -> 0
    // saved_rbp           -> 0
    // saved_rdi           -> 0
    // saved_rsi           -> 0
    // saved_r12           -> pointer to fiber_entry_thunk
    // saved_r13           -> this (pointer to Fiber instance passed as arg to entry thunk)
    // saved_r14           -> 0
    // saved_r15           -> 0
    // saved_rflags        -> 0x202 (interrupts enabled)
    // saved_teb_stackbase -> ctx_.stack_base
    // saved_teb_stacklimit-> ctx_.stack_limit
    // saved_xmm6..xmm15   -> 10 * 16 bytes = 160 bytes of zero
    // [RSP points here, must be 16-byte aligned before 'ret']

    auto* sp = static_cast<std::uintptr_t*>(ctx_.stack_base);

    // Ensure 16-byte alignment before ret pushes/pops
    // In Windows x64, at function entry (before RET execution), RSP % 16 == 8
    // So right after RET returns to trampoline, RSP % 16 == 0.
    sp = reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(sp) & ~0xF);

    // Trampoline return address
    *(--sp) = reinterpret_cast<std::uintptr_t>(&minfiber_trampoline);

    // GPRs: rbx, rbp, rdi, rsi, r12, r13, r14, r15
    *(--sp) = 0; // rbx
    *(--sp) = 0; // rbp
    *(--sp) = 0; // rdi
    *(--sp) = 0; // rsi
    *(--sp) = reinterpret_cast<std::uintptr_t>(&fiber_entry_thunk); // r12
    *(--sp) = reinterpret_cast<std::uintptr_t>(this);               // r13
    *(--sp) = 0; // r14
    *(--sp) = 0; // r15

    // RFLAGS (0x202: IF flag set)
    *(--sp) = 0x202;

    // TEB stack boundaries
    *(--sp) = reinterpret_cast<std::uintptr_t>(ctx_.stack_base);
    *(--sp) = reinterpret_cast<std::uintptr_t>(ctx_.stack_limit);

    // XMM registers: 160 bytes (10 * 16 bytes)
    auto* byte_sp = reinterpret_cast<std::uint8_t*>(sp);
    byte_sp -= 160;
    std::memset(byte_sp, 0, 160);

    ctx_.rsp = byte_sp;
}

} // namespace minfiber
