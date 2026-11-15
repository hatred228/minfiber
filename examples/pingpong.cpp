// path: examples/pingpong.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/scheduler.hpp"
#include <iostream>

void player(const std::string& name, int count) {
    for (int i = 1; i <= count; ++i) {
        std::cout << "[" << name << "] Hit ball #" << i << "!\n";
        minfiber::yield();
    }
    std::cout << "[" << name << "] Done playing.\n";
}

int main() {
    std::cout << "=== MinFiber Cooperative Ping-Pong Demo ===\n\n";

    minfiber::Scheduler sched;

    sched.spawn([] { player("Ping", 5); }, 64 * 1024, "PingWorker");
    sched.spawn([] { player("Pong", 5); }, 64 * 1024, "PongWorker");

    std::cout << "[Main] Starting fiber scheduler...\n";
    sched.run();
    std::cout << "[Main] All fibers finished execution.\n";

    return 0;
}
