#pragma once
#include <stddef.h>

void pmm_init();

#ifdef __cplusplus
extern "C" {
#endif
void * malloc(size_t size);
void free(void *mem);
#ifdef __cplusplus
}
#endif