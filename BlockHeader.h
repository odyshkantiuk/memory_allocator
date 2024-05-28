#pragma once
#include <windows.h>

struct BlockHeader {
    size_t size;
    bool is_free;
    bool is_first;
    bool is_last;
};