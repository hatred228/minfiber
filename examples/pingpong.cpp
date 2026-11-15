// path: examples/pingpong.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/scheduler.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void player(const std::string& name, int count) {
    for (int i = 1; i <= count; ++i) {
        std::cout << "[" << name << "] -> Hit ball #" << i << "!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        minfiber::yield();
    }
    std::cout << "[" << name << "] Done playing.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  MinFiber: Hardware Coroutine Switch   \n";
    std::cout << "========================================\n\n";

    minfiber::Scheduler sched;

    sched.spawn([] { player("Ping", 5); }, 64 * 1024, "PingWorker");
    sched.spawn([] { player("Pong", 5); }, 64 * 1024, "PongWorker");

    std::cout << "[Main] Starting fiber scheduler on single OS thread...\n\n";
    sched.run();
    std::cout << "\n[Main] All fibers finished execution cleanly.\n";
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}
