// idk what lib name that contains cpp runtime is
#include <libcpp.hpp>
#include <libc.hpp>

void Console::operator <<(int val) {
    printf("%d", val);
}

void Console::operator<<(const char *val) {
    printf(val);
}

void Console::operator<<(char val) {
    printf("%s", val);
}

void Console::operator<<(char *val) {
    printf("%s", val);
}
void test() {
    con << 123;
    con << " testing cpp\n";
    con << "this is a const char *, ";
    con << (char *)"this is a char *\n";
}