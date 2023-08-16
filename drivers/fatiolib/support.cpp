#include <libc.hpp>
extern void print_char(char c);
extern "C" {
    void printf2(const char * s, ...) {
        va_list ap;
        va_start(ap, s);
        vsprintf(NULL, print_char, s, ap);
        va_end(ap);
    }
}