// path: examples/generator.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/scheduler.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "========================================\n";
    std::cout << "  MinFiber: Cooperative Generators Demo \n";
    std::cout << "========================================\n\n";

    minfiber::Scheduler sched;

    sched.spawn([] {
        uint64_t a = 0, b = 1;
        for (int i = 0; i < 8; ++i) {
            std::cout << "[Fibonacci Fiber] Term " << i << " = " << a << "\n";
            uint64_t next = a + b;
            a = b;
            b = next;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            minfiber::yield();
        }
    }, 64 * 1024, "Fibonacci");

    sched.spawn([] {
        for (int i = 1; i <= 8; ++i) {
            std::cout << "  [Factorial Fiber] Step " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            minfiber::yield();
        }
    }, 64 * 1024, "Factorial");

    sched.run();

    std::cout << "\n[Main] Computation complete.\n";
    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
