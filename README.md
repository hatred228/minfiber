# minfiber

A lightweight, high-performance cooperative coroutine (fiber) runtime written in C++20 and x86_64 assembly (MASM) for Windows.

## Overview

`minfiber` demonstrates manual thread context switching at the CPU register level, bypassing the operating system kernel scheduler. It allocates custom execution stacks, manipulates RSP and callee-saved registers directly, and adheres strictly to the Microsoft x64 Calling Convention and Thread Environment Block (TEB) requirements.

### Key Features

- **Assembly Context Switching (`switch_x64.asm`)**:
  - Preserves all 8 general-purpose callee-saved registers (`RBX`, `RBP`, `RDI`, `RSI`, `R12`, `R13`, `R14`, `R15`).
  - Preserves `RFLAGS`.
  - Preserves callee-saved SIMD registers (`XMM6` through `XMM15`).
  - Saves and restores TEB stack boundaries (`StackBase` at `GS:[08h]`, `StackLimit` at `GS:[10h]`) to prevent stack checks and SEH corruption.
- **Protected Virtual Memory Stacks**:
  - Stack allocated via `VirtualAlloc` aligned to system page granularity.
  - Guard page (`PAGE_NOACCESS`) at the bottom of the stack to trap stack overflows cleanly.
- **Cooperative Scheduler**:
  - FIFO round-robin fiber scheduling with non-preemptive `yield()` and `spawn()`.
  - Trampoline entry mechanism with 32-byte shadow space alignment.

## Project Structure

```text
minfiber/
├── CMakeLists.txt
├── README.md
├── include/
│   └── minfiber/
│       ├── fiber.hpp        # Fiber abstraction and stack frame setup
│       └── scheduler.hpp    # Cooperative scheduler & public API
├── src/
│   ├── switch_x64.asm       # Core MASM context switch & entry trampoline
│   ├── fiber.cpp            # VirtualAlloc / guard page & initial stack forge
│   └── scheduler.cpp        # Dispatcher loop and yield coordination
└── examples/
    ├── pingpong.cpp         # Ping-pong yield benchmark / demo
    └── generator.cpp        # Cooperative concurrent generators
```

## Context Switch Mechanics (x86_64)

When `minfiber_switch(from_rsp, to_rsp)` is invoked:

```text
High Memory
+-----------------------+
| Return Address (RIP)  | <- Pushed by CALL
+-----------------------+
| RBX, RBP, RDI, RSI    |
| R12, R13, R14, R15    | <- 8 GPRs (64 bytes)
+-----------------------+
| RFLAGS                | <- PUSHFQ (8 bytes)
+-----------------------+
| TEB StackBase & Limit | <- 16 bytes
+-----------------------+
| XMM6 - XMM15          | <- 10 x 16 = 160 bytes
+-----------------------+ <- RSP points here during switch
Low Memory
```

## Building with CMake & MSVC

Requirements:
- Windows 10/11 x64
- Visual Studio 2022 (with "Desktop development with C++" and MASM enabled)
- CMake 3.20+

```powershell
# In project root
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```

### Running Examples

```powershell
.\Release\demo_pingpong.exe
.\Release\demo_generator.exe
```

## License

MIT License
