// VMWare SVGA Driver
#include <io.h>
#include <logging.hpp>
#include <vmsvga.hpp>
#include <pci.hpp>
#include <libc.hpp>

static Logging log("VMSVGA");
pci_dev_t svga_dev;
static uint16_t port_base = 0;

uint32_t *fb_start = 0;
uint32_t *fifo_start = 0;
uint32_t fifo_size = 0;

void svga_write_reg(uint32_t reg, uint32_t val) {
    outl(port_base+SVGA_INDEX, reg);
    outl(port_base+SVGA_VALUE, val);
}

uint32_t svga_read_reg(uint32_t reg) {
    outl(port_base+SVGA_INDEX, reg);
    return inl(port_base+SVGA_VALUE);
}

uint32_t svga_read_fifo(uint32_t reg) {
    return fifo_start[reg];
}

void svga_write_fifo(uint32_t data) {
    fifo_start[svga_read_fifo(SVGA_FIFO_NEXT_CMD)] = data;
}
void putpixel(int x, int y, int color);
void VMSVGA::Init() {
    log.info("Starting...\n");
    svga_dev = PCI::Get(0x15AD, 0x0405, -1);
    if (svga_dev.device_num == 0) {
        log.info("No device found.\n");
        return;
    }
    PCI::BusMasterEnable(svga_dev);
    log.info("Found VMWARE SVGA Device.\n");
    port_base = PCI::Read(svga_dev, PCI_BAR0) - 1;
    log.info("Port base: 0x%x\n", port_base);
    svga_write_reg(SVGA_REG_ID, 0x90000002);
    if (svga_read_reg(SVGA_REG_ID) != 0x90000002) {
        log.info("Not capable.\n");
        return;
    }
    fb_start = (uint32_t *)svga_read_reg(SVGA_REG_FB_START);
    fifo_start = (uint32_t *)svga_read_reg(SVGA_REG_FIFO_START);
    fifo_size = svga_read_reg(SVGA_REG_FIFO_SIZE);
    fifo_start[SVGA_FIFO_MIN] = 293 * 4;
    fifo_start[SVGA_FIFO_NEXT_CMD] = 293 * 4;
    fifo_start[SVGA_FIFO_STOP] = 293 * 4;
    fifo_start[SVGA_FIFO_MAX] = fifo_size;
    svga_write_reg(SVGA_REG_CONFIG_DONE, 1);
    //svga_write_reg(SVGA_REG_ENABLE, 1);
    log.info("Info: \n");
    log.info("Max width: %u\n", svga_read_reg(SVGA_REG_MAX_WIDTH));
    log.info("Max height: %u\n", svga_read_reg(SVGA_REG_MAX_HEIGHT));
    log.info("FIFO Size: %u\n", fifo_size);
}
void putpixel(int x, int y, int color) {
    uint32_t *pixel = (uint32_t*)(svga_read_reg(SVGA_REG_FB_START) + y*(svga_read_reg(SVGA_REG_HEIGHT)*svga_read_reg(SVGA_REG_FB_OFFEST)) + x*svga_read_reg(SVGA_REG_FB_OFFEST));
    *pixel = color;
}
void VMSVGA::SetMode(uint32_t w, uint32_t h, uint32_t bpp) {
    svga_write_reg(SVGA_REG_BPP, bpp);
    svga_write_reg(SVGA_REG_WIDTH, w);
    svga_write_reg(SVGA_REG_HEIGHT, h);
    svga_write_reg(SVGA_REG_ENABLE, 1);
}