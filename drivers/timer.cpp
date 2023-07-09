#include <idt.hpp>
#include <io.h>
#include <timer.hpp>

int timer_tick = 0;

void timer_idt(registers_t *regs) {
    timer_tick++;
}

void Timer::Init() {
    IDT::AddHandler(0, timer_idt);
    int divisor = 1193180 / 100;
    outb(0x43, 0x36);
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}