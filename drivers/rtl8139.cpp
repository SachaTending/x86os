#include <io.h>
#include <pci.hpp>
#include <logging.hpp>
#include <rtl8139.hpp>
#include <idt.hpp>
#include <libc.hpp>
#include <common.h>

static Logging log("RTL8139");

static pci_dev_t rtl_dev;

static uint16_t io_base;
static uint32_t bar0;
static uint32_t irq;

static uint32_t rx_buffer[8192+16+1500];
static uint8_t mac_addr[5];

static uint32_t current_packet_ptr;
static uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
static uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};

#define RX_BUF_SIZE 8192
typedef struct ethernet_frame {
  uint8_t dst_mac_addr[6];
  uint8_t src_mac_addr[6];
  uint16_t type;
  uint8_t data[];
} __attribute__((packed)) ethernet_frame_t;
#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IP  0x0800
static void receive_packet() {
    uint8_t tmp_cmd = inb(io_base+0x37);
    if (tmp_cmd & 0x01) {
        log.info("buffer cleared.\n");
        return;
    }
    uint16_t * t = (uint16_t*)(&rx_buffer + current_packet_ptr);
    // Skip packet header, get packet length
    uint16_t packet_length = *(t + 1);
    // Skip, packet header and packet length, now t points to the packet data
    t = t + 2;
    //qemu_printf("Printing packet at addr 0x%x\n", (uint32_t)t);
    //xxd(t, packet_length);

    // Now, ethernet layer starts to handle the packet(be sure to make a copy of the packet, insteading of using the buffer)
    // and probabbly this should be done in a separate thread...
    ethernet_frame_t *pack = (ethernet_frame_t *)t;
    log.info("dst mac: %01x:%01x:%01x:%01x:%01x:%01x\n", 
        pack->dst_mac_addr[0], 
        pack->dst_mac_addr[1], 
        pack->dst_mac_addr[2], 
        pack->dst_mac_addr[3], 
        pack->dst_mac_addr[4], 
        pack->dst_mac_addr[5]
    );
    log.info("src mac: %02x:%02x:%02x:%02x:%02x:%02x\n", 
        pack->src_mac_addr[0], 
        pack->src_mac_addr[1], 
        pack->src_mac_addr[2], 
        pack->src_mac_addr[3], 
        pack->src_mac_addr[4], 
        pack->src_mac_addr[5]
    );
    log.info("type: %u\n", pack->type);
    current_packet_ptr = (current_packet_ptr + packet_length + 4 + 3) & ~3;

    if(current_packet_ptr > RX_BUF_SIZE)
        current_packet_ptr -= RX_BUF_SIZE;

    outw(io_base + 0x38, current_packet_ptr - 0x10);
}

static void idt_handl(registers_t *regs) {
    UNUSED(regs);
    uint16_t status = inw(io_base+0x3e);
    if (status & (1<<2)) {
        log.info("Packed sended.\n");
    } else if (status & (1<<0)) {
        log.info("Packed received\n");
        receive_packet();
    }
    outw(io_base + 0x3E, 0x5);
}
static void read_mac_addr() {
    uint32_t mac_part1 = inw(io_base+0x00);
    uint16_t mac_part2 = inw(io_base+0x04);
    mac_addr[0] = mac_part1 >> 0;
    mac_addr[1] = mac_part1 >> 8;
    mac_addr[2] = mac_part1 >> 16;
    mac_addr[3] = mac_part1 >> 24;

    mac_addr[4] = mac_part2 >> 0;
    mac_addr[5] = mac_part2 >> 8;
    log.info("MAC Address: %01x:%01x:%01x:%01x:%01x:%01x\n", 
        mac_addr[0], 
        mac_addr[1], 
        mac_addr[2], 
        mac_addr[3], 
        mac_addr[4], 
        mac_addr[5]
    );
}
void RTL8139::Init() {
    log.info("Searching card...\n");
    rtl_dev = PCI::Get(0x10EC, 0x8139, -1);
    if (rtl_dev.device_num == 0) {
        log.info("Card not found.\n");
        return;
    }
    bar0 = PCI::Read(rtl_dev, PCI_BAR0);
    io_base = bar0 & (~0x3);
    log.info("I/O Base: 0x%x\n", io_base);
    PCI::BusMasterEnable(rtl_dev);
    outb(io_base+0x52, 0);
    outb(io_base+0x37, 0x10);
    while ((inb(io_base+0x37) & 0x10) != 0)
    {
        // wait for init
    }
    outl(io_base+0x30, (uint32_t)&rx_buffer);
    outw(io_base + 0x3C, 0x0005);
    outl(io_base+0x44, 0xf | (1 << 7));
    outb(io_base+0x37, 0x0C);
    irq = PCI::Read(rtl_dev, PCI_INTERRUPT_LINE);
    IDT::AddHandler(irq, idt_handl);
    read_mac_addr();
}