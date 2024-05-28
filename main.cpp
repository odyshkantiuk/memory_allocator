#include "MemoryAllocator.h"
#include "Tester.h"


int main() {
    default_arena_size = 4096;

    mem_alloc(2001);
    mem_print();
    mem_alloc(20000);
    mem_print();
    void* p1 = mem_alloc(144);
    void* p2 = mem_alloc(144);
    mem_print();
    mem_realloc(p1, 288);
    mem_free(p2);
    mem_print();
    /*
    tester((size_t) 3, (size_t) 1024);
     */
    return 0;
}