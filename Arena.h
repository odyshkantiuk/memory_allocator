#pragma once
#include <windows.h>

struct Arena {
    size_t size;
    void* base;
    Arena* next;
};