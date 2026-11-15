// path: examples/generator.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/scheduler.hpp"
#include <iostream>

int main() {
    std::cout << "=== MinFiber Generator / Cooperative Tasks ===\n\n";

    minfiber::Scheduler sched;

    sched.spawn([] {
        uint64_t a = 0, b = 1;
        for (int i = 0; i < 10; ++i) {
            std::cout << "[Fibonacci Fiber] Term " << i << " = " << a << "\n";
            uint64_t next = a + b;
            a = b;
            b = next;
            minfiber::yield();
        }
    }, 64 * 1024, "Fibonacci");

    sched.spawn([] {
        for (int i = 1; i <= 10; ++i) {
            std::cout << "  [Factorial Fiber] " << i << "! step\n";
            minfiber::yield();
        }
    }, 64 * 1024, "Factorial");

    sched.run();

    std::cout << "\n[Main] Computation complete.\n";
    return 0;
}
