// path: include/minfiber/scheduler.hpp
// context: C++20 / minfiber / windows x64
#pragma once

#include "fiber.hpp"
#include <deque>
#include <memory>
#include <vector>

namespace minfiber {

class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    static Scheduler& current();

    void spawn(FiberFunction fn, std::size_t stack_size = 64 * 1024, std::string name = "");
    void yield();
    void run();

    [[nodiscard]] Fiber* running_fiber() const noexcept { return current_fiber_; }

    void on_fiber_exit();

private:
    void switch_to(Fiber* target);

    std::vector<std::unique_ptr<Fiber>> all_fibers_;
    std::deque<Fiber*> ready_queue_;
    std::unique_ptr<Fiber> main_fiber_;
    Fiber* current_fiber_{nullptr};
    bool running_{false};
};

inline void yield() {
    Scheduler::current().yield();
}

inline void spawn(FiberFunction fn, std::size_t stack_size = 64 * 1024, std::string name = "") {
    Scheduler::current().spawn(std::move(fn), stack_size, std::move(name));
}

} // namespace minfiber
