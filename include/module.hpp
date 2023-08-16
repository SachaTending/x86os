#pragma once

enum MOD_TYPE {
    NEED_PCI
};

typedef struct 
{
    const char *name;
    void(*entry)();
    MOD_TYPE type;
} module_t;
#define SECTION( S ) __attribute__ ((section ( S )))
#define ADD_MODULE(entry2, name2, type2) static volatile SECTION(".modules") module_t mod { .name = name2, .entry = entry2, .type = type2 }