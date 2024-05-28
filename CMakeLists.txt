cmake_minimum_required(VERSION 3.27)
project(memory_allocator)

set(CMAKE_CXX_STANDARD 17)

add_executable(memory_allocator main.cpp
        MemoryAllocator.cpp
        MemoryAllocator.h
        BlockHeader.h
        Arena.h
        Tester.cpp
        Tester.h
)
