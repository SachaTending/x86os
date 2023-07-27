#include <graphics.hpp>
#include <mouse.hpp>
#include <idt.hpp>
#include <io.h>
#include <common.h>
#include <logging.hpp>
#include <graphics.hpp>

extern void svga_flush();

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
extern fbinfo_t *fb_info;

int old_x, old_y;
int csize = 10;
#define MOUSE_RIGHT_BUTTON(flag) (flag & 0x2)
#define MOUSE_LEFT_BUTTON(flag) (flag & 0x1)
#define MOUSE_MIDDLE_BUTTON(flag) (flag & 0x4)
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
            old_x = x;
            old_y = y;
            x += bytes[1];
            y -= bytes[2];
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (((int)fb_info) != 0) {
                if ((y + csize) > fb_info->height) y = fb_info->height - csize;
                if ((x + csize) > fb_info->width) x = fb_info->width - csize;
            }
            iter = 0;
            if (((int)fb_info) != 0) {
                if (not MOUSE_LEFT_BUTTON(bytes[0]))Graphics::Square_Filled(old_x, old_y, old_x+csize, old_y+csize, 0);
                if (MOUSE_RIGHT_BUTTON(bytes[0])) csize-=10;
                else if (MOUSE_MIDDLE_BUTTON(bytes[0])) csize+=10;
                if (csize > 50) csize = 50;
                if (csize < 10) csize = 10;
                if (MOUSE_LEFT_BUTTON(bytes[0])) Graphics::Square_Filled(x, y, x+csize, y+csize, 100);
                else Graphics::Square_Filled(x, y, x+csize, y+csize, 500);
                svga_flush();
            }
            log.info("x: %d y: %d csize: %d\n", x,y,csize);
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