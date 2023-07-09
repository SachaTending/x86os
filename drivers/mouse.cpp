#include <graphics.hpp>
#include <mouse.hpp>
#include <idt.hpp>
#include <io.h>
#include <common.h>
#include <logging.hpp>

static Logging log("Mouse");

static int x, y;

static void wait() {
    int t = 10000000;
    while (t--) {
        if ((inb(0x64) & 2)==0) {
            return;
        }
    }
}

static void wait2() {
    int t = 10000000;
    while (t--) {
        if ((inb(0x64) & 1)==0) {
            return;
        }
    }
}
static void write(uint8_t data) {
    wait();
    outb(0x64, 0xD4);
    wait();
    outb(0x60, data);
}

static uint8_t read() {
    wait();
    return inb(0x60);
}

static void idt_handl(registers_t *regs) {
    UNUSED(regs);
    static uint8_t iter = 0;
    static char bytes[3];
    switch (iter)
    {
        case 0:
            bytes[0] = read();
            iter++;
            break;
        case 1:
            bytes[1] = read();
            iter++;
            break;
        case 2:
            bytes[2] = read();
            x += bytes[1];
            y -= bytes[2];
            log.info("x: %d y: %d\n", x,y);
    }
}


void Mouse::Init() {
    uint8_t status;
    wait();
    outb(0x64, 0xAB);
    wait();
    outb(0x64, 0x20);
    wait2();
    status = (inb(0x60) | 2);
    wait();
    outb(0x64, 0x60);
    wait();
    outb(0x60, status);
    write(0xF6);
    read();
    write(0xF4);
    read();
    IDT::AddHandler(12, idt_handl);
    log.info("Initialized.\n");
}