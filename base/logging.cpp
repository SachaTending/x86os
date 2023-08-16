// Logging system, bcz i need it
#include <terminal.hpp>
#include <logging.hpp>
#include <libc.hpp>

extern int timer_tick;

Logging::Logging(const char *log_name) {
    this->log_name = log_name;
}
extern "C" void print_char(char c);
void Logging::info(const char *text, ...) {
    printf("[%s][%d][INFO]: ", this->log_name, timer_tick);
    va_list ap;
    va_start(ap, text);
    vsprintf(NULL, print_char, text, ap);
    va_end(ap);
}