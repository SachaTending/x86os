#include <io.h>
#include <idt.hpp>
#include <libc.hpp>
#include <int32.h>
#include <graphics.hpp>
#include <libc.hpp>
#include <flanterm.h>
#include <fb2.h>

char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b',   /* Backspace */
    '\t',           /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       /* Enter key */
    0,          /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
    '\'', '`',   0,     /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 49 */
    'm', ',', '.', '/',   0,                    /* Right shift */
    '*',
    0,  /* Alt */
    ' ',    /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
char buf[1024];
int buf_loc = 0;

flanterm_context *ctx;

void cls_buff() {
    for (int i=0;i<1024;i++) {
        buf[i] = '\0';
    }
    buf_loc = 0;
    printf("> ");
}

uint32_t *vram;

void vesa_test();
void memmap_print();
struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));
vbe_mode_info_structure *mode_info = (vbe_mode_info_structure *)0x7c000;
static void putpixel( int x,int y, int color) {
    //unsigned where = (y * mode_info->pitch) + (x * (mode_info->bpp / 8));
    unsigned where = x + y * (mode_info->pitch / sizeof(uint32_t));
    //uint32_t *where = (uint32_t *)mode_info->framebuffer;
    //int row = (y * mode_info->pitch) / 4;
    //where[row + x] = color;
    //vram[where] = color & 0x00ff0000;              // BLUE
    //vram[where + 1] = color & 0x0000ff00;   // GREEN
    //vram[where + 2] = color & 0x000000ff;  // RED
    vram[where] = color;
    vram[where + 1] = color;
    vram[where + 2] = color;
    vram[where + 3] = color;
    //vram[x*y] = color;
}
void draw_text(char * text, int start_x, int start_y);

void gui_draw(const char *text, int x, int y) {
    int len = strlen(text);
    for (int i=0;i<len;i++) {
        draw_text((char *)&(text[i]), i*8, y);
    }
}
int screen_x, screen_y;

bool in_vesa = false;
void drawchar(unsigned char c, int x, int y, int fgcolor, int bgcolor);
extern int fx, fy;
#define font_height fy
extern bool terminal_disabled;
unsigned short mode;
uint16_t find_mode(int x, int y, int bpp) {
	terminal_disabled = true;
	regs16_t r;
	uint16_t mode2;
	while (mode2 != 0xffff) {
		r.ax = 0x4F01;
        r.cx = mode2;
        r.es = seg(mode_info);
        r.di = off(mode_info);
		int32(0x10, &r);
		if (r.ax == 0x004F) {
			terminal_disabled = true;
			//printf("0x%x: width: %u height: %u bpp: %u\n", mode2, mode_info->width, mode_info->height, mode_info->bpp);
			if (mode_info->width == x && mode_info->height == y && mode_info->bpp == bpp) {
				printf("0x%x: width: %u height: %u bpp: %u\n", mode2, mode_info->width, mode_info->height, mode_info->bpp);
				mode = mode2;
				return mode2;
			}
		}
		memset(&r, 0, sizeof(regs16_t));
		mode2++;
	}
	terminal_disabled = false;
	return 0;
}

void putchar_psf(
    /* note that this is int, not char as it's a unicode character */
    unsigned short int c,
    /* cursor position on screen, in characters not in pixels */
    int cx, int cy,
    /* foreground and background colors, say 0xFFFFFF and 0x000000 */
    uint32_t fg, uint32_t bg);
void gui_putchar(char c) {
    if (!in_vesa) return;
    if (c == '\n') {
        screen_y += fy;
        screen_x = 0;
        goto end;
    }
    if (screen_x + fx > mode_info->width) {
        screen_y += fy;
        screen_x = 0;
    }
    if (screen_y + fy > mode_info->height) {
        screen_x = 0;
        screen_y = 0;
    }
    //draw_text(&c, screen_x, screen_y);
    putchar_psf(c, screen_x, screen_y, 0xffffffff, 0x0);
    screen_x += fx;
end:
    return;
}

extern uint8_t builtin_font[];

void gui_print(const char *text) {
    int len = strlen(text);
    for (int i=0;i<len;i++) {
        gui_putchar(text[i]);
    }
}
extern uint8_t * get_font_array();
//unsigned short mode = 0x144 | 0x4000;
void vesa_info() {
    regs16_t r;
	(void)r;
    r.ax = 0x4F01;
    r.cx = mode;
    r.es = seg(mode_info);
    r.di = off(mode_info);
    int32(0x10, &r);
    if (r.ax == 0x004F) {
        printf("%dx%dx%d\n", mode_info->width, mode_info->height, mode_info->bpp);
    } else {
        printf("err 0x%x\n", r.ax);
    }
}

void cls() {
    for (int x=0;x<mode_info->width;x++) {
        for (int y=0;y<mode_info->height;y++) {
            putpixel(x, y, 0);
        }
    }
    screen_x = 0;
    screen_y = 0;
}

void vesa_modes() {
    regs16_t r;

}
const char msg[] = "Hello world\n";
void psf_init();

void vesa_set() {
	find_mode(1024, 768, 32);
	printf("mode: 0x%x\n", mode);
	if (mode == 0) {
		printf("no mode found\n");
		return;
	}
    regs16_t r;
    r.ax = 0x4F02;
    r.bx = mode;
    int32(0x10, &r);
    if (r.ax == 0x004F) {
        printf("ok\n");
        memset((void *)&r, 0, sizeof(regs16_t));
        r.ax = 0x4F01;
        r.cx = mode;
        r.es = seg(mode_info);
        r.di = off(mode_info);
        int32(0x10, &r);
        printf("0x%x\n", mode_info->framebuffer);
        if (r.ax == 0x004F) {
			psf_init();
            printf("ok\n");
			static fbinfo_t finfo;
			finfo.height = mode_info->height;
            finfo.width = mode_info->width;
            finfo.pitch = mode_info->pitch;
            Graphics::Init(putpixel, &finfo);
			putchar_psf('>', 1, 1, 0xffffffff, 0x0);
            vram = (uint32_t *)mode_info->framebuffer;
            //Graphics::Square_Filled(100, 100, 150, 150, 123);
            printf("%dx%dx%d\n", mode_info->width, mode_info->height, mode_info->bpp);
            in_vesa = true;
        }
    } else {
        printf("err 0x%x\n", r.ax);
    }
}

void kbd_int(registers_t *regs) {
	(void)regs;
    uint8_t scancode = inb(0x60);
    char c = kbdus[scancode];
    if (scancode & 0x80) {
        return;
    } else {
        printf("%c", c);
        if (c == '\n') {
            // Handle commands
            if (!strcmp("vesa", (char *)&buf)) {
                vesa_test();
            } else if (!strcmp("memmap", (char *)&buf)) {
                memmap_print();
            } else if (!strcmp("set", (char *)&buf)) {
                vesa_set();
            } else if (!strcmp("info", (char *)&buf)) {
                vesa_info();
            } else if (!strcmp("cls", (char *)&buf)) {
                cls();
            }
            cls_buff();
        } else {
            buf[buf_loc++] = c;
        }
    }
}

void kbd_init() {
    IDT::AddHandler(1, kbd_int);
    for (int i=0;i<1024;i++) {
        buf[i] = '\0';
    }
    buf_loc = 0;
}