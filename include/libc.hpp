#pragma once
#include <stdint.h>

typedef __builtin_va_list va_list;

#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)


#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
#define va_start(v, ...)	__builtin_va_start(v, 0)
#else
#define va_start(v,l)	__builtin_va_start(v,l)
#endif

#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L \
    || __cplusplus + 0 >= 201103L
#define va_copy(d,s)	__builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)	__builtin_va_copy(d,s)

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif
void *memcpy(void *dst, void const *src, int n);
uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count);
void *memset(void *dst,char val, int n);
int memcmp(uint8_t * data1, uint8_t * data2, int n);

int strcpy(char *dst,const char *src);
int strlen(const char* str);
int strcmp(const char *dst, char *src);
int strncmp( const char* s1, const char* s2, int c );

int atoi(char * string);
void itoa(char *buf, unsigned long int n, int base);

int isspace(char c);
int isprint(char c);

void vsprintf(char * str, void (*putchar)(char), const char * format, va_list arg);
void vsprintf_helper(char * str, void (*putchar)(char), const char * format, uint32_t * pos, va_list arg);
void printf(const char * s, ...);
#ifdef __cplusplus
}
#endif