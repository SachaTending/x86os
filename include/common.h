#pragma once

#define UNUSED(x) (void)x
void crash1(const char *file, int line, const char *func);
void crash2(const char *fmt, ...);

#define CRASH(x, ...) crash1(__FILE__, __LINE__, __func__);crash2(x)