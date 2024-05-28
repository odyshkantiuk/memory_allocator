#include "MemoryAllocator.h"
#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <iomanip>

// Global variables for default arena size, list of arenas and block map
size_t default_arena_size;
Arena* arena_list = nullptr;
std::unordered_map<void*, BlockHeader*> block_map;

// Function to allocate memory of given size
void* mem_alloc(size_t size) {
    // If size is 0, return nullptr
    if (size == 0) {
        return nullptr;
    }
    size = (size + 3) & ~3;
    // Iterate over all arenas
    for (Arena* arena = arena_list; arena; arena = arena->next) {
        // Base pointer for current arena
        char* base = static_cast<char*>(arena->base);
        // Iterate over all blocks in current arena
        while (reinterpret_cast<size_t>(base) < reinterpret_cast<size_t>(arena->base) + arena->size) {
            // Current block
            auto* block = reinterpret_cast<BlockHeader*>(base);
            // If block is free and its size is enough for allocation
            if (block->is_free && block->size >= size) {
                // Remaining size after allocation
                size_t remaining_size = block->size - size;
                // If remaining size is enough for a new block
                if (remaining_size > sizeof(BlockHeader) + 4) {
                    // Create a new block in the remaining space
                    auto* new_block = reinterpret_cast<BlockHeader*>(base + sizeof(BlockHeader) + size);
                    new_block->size = remaining_size - sizeof(BlockHeader);
                    new_block->is_free = true;
                    new_block->is_first = false;
                    new_block->is_last = block->is_last;
                    // Update size and last flag of current block
                    block->size = size;
                    block->is_last = false;
                    // Add new block to block map
                    block_map[new_block] = new_block;
                }
                // Mark current block as used
                block->is_free = false;
                // Return pointer to allocated memory
                return base + sizeof(BlockHeader);
            }
            // Move to next block
            base += sizeof(BlockHeader) + block->size;
        }
    }
    // If no suitable block was found, create a new arena
    size_t arena_size = std::max(size + sizeof(BlockHeader), default_arena_size);
    void* base = VirtualAlloc(nullptr, arena_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!base) {
        return nullptr;
    }
    // Create a new arena and add it to the list of arenas
    auto* arena = new Arena{ arena_size, base, arena_list };
    arena_list = arena;
    // Create an initial block in the new arena
    auto* initial_block = reinterpret_cast<BlockHeader*>(base);
    initial_block->size = arena_size - sizeof(BlockHeader);
    initial_block->is_free = true;
    initial_block->is_first = true;
    initial_block->is_last = true;
    block_map[base] = initial_block;
    // Base pointer for new arena
    char* base_ptr = static_cast<char*>(base);
    auto* block = reinterpret_cast<BlockHeader*>(base_ptr);
    // Remaining size after allocation
    size_t remaining_size = block->size - size;
    // If remaining size is enough for a new block
    if (remaining_size > sizeof(BlockHeader) + 4) {
        // Create a new block in the remaining space
        auto* new_block = reinterpret_cast<BlockHeader*>(base_ptr + sizeof(BlockHeader) + size);
        new_block->size = remaining_size - sizeof(BlockHeader);
        new_block->is_free = true;
        new_block->is_first = false;
        new_block->is_last = block->is_last;
        block->size = size;
        block->is_last = false;
        // Add new block to block map
        block_map[new_block] = new_block;
    }
    // Mark current block as used
    block->is_free = false;
    // Return pointer to allocated memory
    return base_ptr + sizeof(BlockHeader);
}

// Function to free memory at given pointer
void mem_free(void* ptr) {
    // If ptr is nullptr, do nothing
    if (!ptr) {
        return;
    }
    // Find block corresponding to ptr in block map
    auto it = block_map.find(static_cast<char*>(ptr) - sizeof(BlockHeader));
    // If block is not found, do nothing
    if (it == block_map.end()) {
        return;
    }
    // Mark block as free
    BlockHeader* block = it->second;
    block->is_free = true;
    // Base pointer for current block
    char* base = reinterpret_cast<char*>(block);
    // Next block
    auto* next_block = reinterpret_cast<BlockHeader*>(base + sizeof(BlockHeader) + block->size);
    // If next block is free, merge it with current block
    if (block_map.find(next_block) != block_map.end() && next_block->is_free) {
        block->size += sizeof(BlockHeader) + next_block->size;
        block->is_last = next_block->is_last;
        // Remove next block from block map
        block_map.erase(next_block);
    }
    // Iterate over all blocks
    for (it= block_map.begin(); it != block_map.end();) {
        BlockHeader* current_block = it->second;
        // If current block is free
        if (current_block->is_free) {
            // Base pointer for current block
            char* current_base = reinterpret_cast<char*>(current_block);
            // Next block
            next_block= reinterpret_cast<BlockHeader *>(current_base + sizeof(BlockHeader) + current_block->size);
            // If next block is free, merge it with current block
            if (block_map.find(next_block) != block_map.end() && next_block->is_free) {
                current_block->size += sizeof(BlockHeader) + next_block->size;
                current_block->is_last = next_block->is_last;
                // Remove next block from block map
                block_map.erase(next_block);
                continue;
            }
        }
        // Move to next block
        ++it;
    }
}

// Function to reallocate memory at given pointer with new size
void* mem_realloc(void* ptr, size_t size) {
    // If ptr is nullptr, allocate new memory
    if (!ptr) {
        return mem_alloc(size);
    }
    size = (size + 3) & ~3;
    // Find block corresponding to ptr in block map
    auto it = block_map.find(static_cast<char*>(ptr) - sizeof(BlockHeader));
    // If block is not found, return nullptr
    if (it == block_map.end()) {
        return nullptr;
    }
    // Get the block from the iterator
    BlockHeader* block = it->second;
    // If the current block size is greater than or equal to the new size
    if (block->size >= size) {
        // Calculate remaining size after allocation
        size_t remaining_size = block->size - size;
        // If remaining size is enough for a new block
        if (remaining_size > sizeof(BlockHeader) + 4) {
            // Create a new block in the remaining space
            auto* new_block = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(block) + sizeof(BlockHeader) + size);
            new_block->size = remaining_size - sizeof(BlockHeader);
            new_block->is_free = true;
            new_block->is_first = false;
            new_block->is_last = block->is_last;
            // Update size and last flag of current block
            block->size = size;
            block->is_last = false;
            // Add new block to block map
            block_map[new_block] = new_block;
        }
        // Return the original pointer
        return ptr;
    }
    // If the current block size is less than the new size, allocate new memory
    void* new_ptr = mem_alloc(size);
    // If memory allocation failed, return nullptr
    if (!new_ptr) {
        return nullptr;
    }
    // Copy old memory content to new memory
    memcpy(new_ptr, ptr, block->size);
    // Free old memory
    mem_free(ptr);
    // Return pointer to new memory
    return new_ptr;
}

void mem_print() {
    int columnWidth = 20;
    int numColumns = 5;
    int separatorWidth = 1;
    int totalWidth = numColumns * columnWidth + (numColumns - 1) * separatorWidth;
    std::cout << std::string(totalWidth, '-') << std::endl;
    std::cout << std::left << std::setw(columnWidth) << "arena/block" << '|'
              << std::setw(columnWidth) << "size" << '|'
              << std::setw(columnWidth) << "first" << '|'
              << std::setw(columnWidth) << "last" << '|'
              << std::setw(columnWidth) << "free" << std::endl;
    std::cout << std::string(totalWidth, '-') << std::endl;
    for (Arena* arena = arena_list; arena; arena = arena->next) {
        std::cout << "Arena - " << arena->size << "b" << std::endl;
        char* base = static_cast<char*>(arena->base);
        while (reinterpret_cast<size_t>(base) < reinterpret_cast<size_t>(arena->base) + arena->size) {
            auto* block = reinterpret_cast<BlockHeader*>(base);
            std::cout << std::left << std::setw(columnWidth) << reinterpret_cast<void*>(base)
                      << '|' << std::setw(columnWidth) << block->size
                      << '|' << std::setw(columnWidth) << (block->is_first ? "true" : "false")
                      << '|' << std::setw(columnWidth) << (block->is_last ? "true" : "false")
                      << '|' << std::setw(columnWidth) << (block->is_free ? "true" : "false") << std::endl;
            base += sizeof(BlockHeader) + block->size;
        }
        std::cout << std::string(totalWidth, '-') << std::endl;
    }
    std::cout << " " << std::endl;
}