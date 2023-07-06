#include <idt.hpp>
#include <io.h>
#include <timer.hpp>

int Timer::Tick = 0;

void timer_idt(registers_t *regs) {
    Timer::Tick++;
}

void Timer::Init() {
    IDT::AddHandler(1, timer_idt);
}