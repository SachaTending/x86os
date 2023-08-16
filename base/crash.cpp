#include <logging.hpp>
#include <libc.hpp>

static Logging log("Kernel"); // simulate kernel logging
extern "C" void print_char(char c);
void fill_bg(uint32_t bg);
void crash1(const char *file, int line, const char *func) {
    fill_bg(0x0000ff);
    log.info("OH NO, KERNEL CRASHED AT: \n");
    log.info("FILE: %s\n", file);
    log.info("LINE: %d\n", line);
    log.info("FUNCTION: %s\n", func);
}

void crash2(const char *fmt, ...) {
    log.info("REASON: ");
    va_list ap;
    va_start(ap, fmt);
    vsprintf(NULL, print_char, fmt, ap);
    va_end(ap);
    log.info("Kernel halted.\n");
    asm volatile ("cli");
    asm volatile ("cld");
    for (;;) asm volatile ("hlt");
    __builtin_unreachable();
}