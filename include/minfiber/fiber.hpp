// path: include/minfiber/fiber.hpp
// context: C++20 / minfiber / windows x64
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace minfiber {

enum class FiberState {
    Ready,
    Running,
    Suspended,
    Terminated
};

using FiberFunction = std::function<void()>;

struct FiberContext {
    void* rsp{nullptr};
    void* stack_base{nullptr};
    void* stack_limit{nullptr};
    std::size_t stack_size{0};
};

class Fiber {
public:
    explicit Fiber(FiberFunction fn, std::size_t stack_size = 64 * 1024, std::string name = "");
    ~Fiber();

    Fiber(const Fiber&) = delete;
    Fiber& operator=(const Fiber&) = delete;

    Fiber(Fiber&& other) noexcept;
    Fiber& operator=(Fiber&& other) noexcept;

    [[nodiscard]] FiberState state() const noexcept { return state_; }
    void set_state(FiberState state) noexcept { state_ = state; }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] FiberContext& context() noexcept { return ctx_; }
    [[nodiscard]] const FiberContext& context() const noexcept { return ctx_; }

    void invoke();

private:
    void allocate_stack(std::size_t size);
    void free_stack();
    void init_stack_frame();

    FiberFunction fn_;
    std::string name_;
    FiberState state_{FiberState::Ready};
    FiberContext ctx_{};
};

} // namespace minfiber
