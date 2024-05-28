#pragma once
#include "Arena.h"
#include "BlockHeader.h"
#include <unordered_map>

extern size_t default_arena_size;
extern Arena* arena_list;
extern std::unordered_map<void*, BlockHeader*> block_map;

void* mem_alloc(size_t size);
void mem_free(void* ptr);
void* mem_realloc(void* ptr, size_t size);
void mem_print();