#include <stddef.h>
#include <io.h>
#include <terminal.hpp>
#include <libc.hpp>
 
/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
void terminal_setcolor(uint8_t color);
void Terminal::Init(void) 
{
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
	terminal_buffer = (uint16_t*) 0xB8000;
    Terminal::Clear();
}

void Terminal::Clear() {
    terminal_row = 0;
	terminal_column = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
void update_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}
void scroll() {
    // Move up
    void * start = (void*)(terminal_buffer + 1 * VGA_WIDTH * 2);
    uint32_t size = terminal_row * VGA_WIDTH * 2;
    if(terminal_row < 25)
        return;
    memcpy(terminal_buffer, start, size);
    // Delete
    start = (void*)(terminal_buffer + size);
    memsetw((uint16_t *)start, terminal_buffer[terminal_row * VGA_WIDTH + terminal_column], VGA_WIDTH);
    terminal_row--;
}

void Terminal::PutChar(char c) 
{
	scroll();
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        goto end;
    }
    if (c == '\r') {
        terminal_column = 0;
        goto end;
    }
    if (c == '\t') {
        terminal_column += 4;
        goto end;
    }
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column >= VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row >= VGA_HEIGHT)
			terminal_row = 0;
	}
	end:
	update_cursor(terminal_column, terminal_row);
}
 
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		Terminal::PutChar(data[i]);
}
 
void Terminal::Print(const char *text) 
{
	terminal_write(text, strlen(text));
}