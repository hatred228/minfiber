cmake_minimum_required(VERSION 3.20)
project(minfiber LANGUAGES C CXX ASM_MASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    enable_language(ASM_MASM)
    add_compile_options(/W4 /WX- /permissive-)
endif()

add_library(minfiber_core STATIC
    src/switch_x64.asm
    src/fiber.cpp
    src/scheduler.cpp
)

target_include_directories(minfiber_core PUBLIC include)

add_executable(demo_pingpong examples/pingpong.cpp)
target_link_libraries(demo_pingpong PRIVATE minfiber_core)

add_executable(demo_generator examples/generator.cpp)
target_link_libraries(demo_generator PRIVATE minfiber_core)
