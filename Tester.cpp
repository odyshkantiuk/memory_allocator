#include "MemoryAllocator.h"
#include <iostream>
#include <cstdint>
#include <vector>

void random_input(void* data, size_t size) {
    uint8_t* bytes = static_cast<uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = rand() % 512;
    }
}

void tester(size_t iterations, size_t max_block_size) {
    default_arena_size = 2048;
    std::vector<BlockHeader*> arenas;
    std::vector<void*> allocations;
    srand(static_cast<unsigned>(time(0)));
    for (size_t i = 0; i < iterations; ++i) {
        std::cout << i + 1 << "/" << iterations << ": \n";
        int operation = rand() % 3;
        switch (operation) {
            case 0: {
                size_t size = rand() % max_block_size + 1;
                std::cout << "mem_alloc(" << size << ")" << std::endl;
                void* ptr = mem_alloc(size);
                if (ptr) {
                    random_input(ptr, size);
                    allocations.push_back(ptr);
                }
                break;
            }
            case 1: {
                if (!allocations.empty()) {
                    size_t index = rand() % allocations.size();
                    void* ptr = allocations[index];
                    std::cout << "mem_free(" << ptr << ")" << std::endl;
                    mem_free(ptr);
                    allocations.erase(allocations.begin() + index);
                }
                break;
            }
            case 2: {
                if (!allocations.empty()) {
                    size_t index = rand() % allocations.size();
                    void* ptr = allocations[index];
                    size_t new_size = rand() % max_block_size + 1;
                    std::cout << "mem_realloc(" << ptr << ", " << new_size << ")" << std::endl;
                    void* new_ptr = mem_realloc(ptr, new_size);
                    if (new_ptr) {
                        random_input(new_ptr, new_size);
                        allocations[index] = new_ptr;
                    }
                }
                break;
            }
        }
        mem_print();
    }
    for (void* ptr : allocations) {
        mem_free(ptr);
    }
    std::cout << "Automatic test completed" << std::endl;
}