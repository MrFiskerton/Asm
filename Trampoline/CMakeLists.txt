cmake_minimum_required(VERSION 3.9)
project(Trampoline)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp
        allocator.cpp
        allocator.hpp
        trampoline.hpp
        argument_types.hpp
        instructions.hpp
        Writer.hpp)

add_executable(Trampoline ${SOURCE_FILES})