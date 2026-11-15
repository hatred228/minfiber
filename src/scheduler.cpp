// path: src/scheduler.cpp
// context: C++20 / minfiber / windows x64
#include "minfiber/scheduler.hpp"

#include <windows.h>
#include <iostream>
#include <cassert>

extern "C" {
    void minfiber_switch(void** from_rsp, void* to_rsp);
    void minfiber_on_exit(void);
}

namespace minfiber {

namespace {
    thread_local Scheduler* t_current_scheduler = nullptr;
}

Scheduler::Scheduler() {
    t_current_scheduler = this;
    main_fiber_ = std::unique_ptr<Fiber>(new Fiber(nullptr, 0, "main"));
    current_fiber_ = main_fiber_.get();
}

Scheduler::~Scheduler() {
    if (t_current_scheduler == this) {
        t_current_scheduler = nullptr;
    }
}

Scheduler& Scheduler::current() {
    assert(t_current_scheduler != nullptr && "No active scheduler in current thread!");
    return *t_current_scheduler;
}

void Scheduler::spawn(FiberFunction fn, std::size_t stack_size, std::string name) {
    auto fiber = std::make_unique<Fiber>(std::move(fn), stack_size, std::move(name));
    ready_queue_.push_back(fiber.get());
    all_fibers_.push_back(std::move(fiber));
}

void Scheduler::yield() {
    if (ready_queue_.empty()) {
        return;
    }

    Fiber* prev = current_fiber_;
    prev->set_state(FiberState::Ready);

    Fiber* next = ready_queue_.front();
    ready_queue_.pop_front();
    ready_queue_.push_back(prev);

    switch_to(next);
}

void Scheduler::run() {
    running_ = true;

    while (!ready_queue_.empty()) {
        Fiber* next = ready_queue_.front();
        ready_queue_.pop_front();
        switch_to(next);
    }

    running_ = false;
    current_fiber_ = main_fiber_.get();
}

void Scheduler::switch_to(Fiber* target) {
    if (target == current_fiber_) {
        return;
    }

    Fiber* prev = current_fiber_;
    current_fiber_ = target;
    target->set_state(FiberState::Running);

    minfiber_switch(&prev->context().rsp, target->context().rsp);
}

void Scheduler::on_fiber_exit() {
    Fiber* exiting = current_fiber_;
    if (exiting) {
        exiting->set_state(FiberState::Terminated);
    }

    if (!ready_queue_.empty()) {
        Fiber* next = ready_queue_.front();
        ready_queue_.pop_front();
        current_fiber_ = next;
        next->set_state(FiberState::Running);
        minfiber_switch(&exiting->context().rsp, next->context().rsp);
    } else {
        current_fiber_ = main_fiber_.get();
        minfiber_switch(&exiting->context().rsp, main_fiber_->context().rsp);
    }
}

} // namespace minfiber

extern "C" void minfiber_on_exit(void) {
    minfiber::Scheduler::current().on_fiber_exit();
}
