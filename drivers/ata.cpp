#include <module.hpp>
#include <io.h>
#include <pci.hpp>
#include <logging.hpp>
#include <ata.hpp>
#include <libc.hpp>

static Logging log("ATA");
static pci_dev_t dev;
uint32_t bmr_cmd=0, bmr_stat=0, bmr_prdt=0;


typedef struct ab {
	uint32_t buffer_phys;
	uint16_t transfer_size;
	uint16_t mark_end;
}__attribute__((packed)) ab;
ab bmr = {};

ata_dsk_t ata_disks[4] = {
    {
        .disk = PRIMARY_MASTER,
        .name = {}
    },
    {
        .disk = PRIMARY_SLAVE,
        .name = {}
    },
    {
        .disk = SECONDARY_MASTER,
        .name = {}
    },
    {
        .disk = SECONDARY_SLAVE,
        .name = {}
    }
};

static uint16_t get_io(ATA_DISK disk) {
    if (disk == PRIMARY_MASTER || disk == PRIMARY_SLAVE) {
        return 0x1F0;
    }
    return 0x170;
}
typedef struct {
	uint16_t flags;
	uint16_t unused1[9];
	char     serial[20];
	uint16_t unused2[3];
	char     firmware[8];
	char     model[40];
	uint16_t sectors_per_int;
	uint16_t unused3;
	uint16_t capabilities[2];
	uint16_t unused4[2];
	uint16_t valid_ext_data;
	uint16_t unused5[5];
	uint16_t size_of_rw_mult;
	uint32_t sectors_28;
	uint16_t unused6[38];
	uint64_t sectors_48;
	uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;
static void setreg(ata_dsk_t dsk, uint16_t reg, uint8_t val) {
    outb(get_io(dsk.disk)+reg, val);
}

static void setbothval(uint16_t reg, uint8_t val) {
    uint8_t old = inb(get_io(PRIMARY_MASTER)+REG_DRIVE);
    outb(get_io(PRIMARY_MASTER)+REG_DRIVE, 0xA << 4);
    outb(get_io(PRIMARY_MASTER)+reg, val);
    outb(get_io(PRIMARY_MASTER)+REG_DRIVE, (0xA + 1) << 4);
    outb(get_io(PRIMARY_MASTER)+reg, val);
    outb(get_io(PRIMARY_MASTER)+REG_DRIVE, old);
    old = inb(get_io(SECONDARY_MASTER)+REG_DRIVE);
    outb(get_io(SECONDARY_MASTER)+REG_DRIVE, 0xA << 4);
    outb(get_io(SECONDARY_MASTER)+reg, val);
    outb(get_io(SECONDARY_MASTER)+REG_DRIVE, (0xA + 1) << 4);
    outb(get_io(SECONDARY_MASTER)+reg, val);
    outb(get_io(SECONDARY_MASTER)+REG_DRIVE, old);
}

static void wait(uint16_t io) {
    for (int i=0;i<4;i++) {
        inb(io+REG_ALT_STATUS);
    }
}

static void soft_rst(ata_dsk_t dsk) {
    uint16_t io = get_io(dsk.disk);
    setreg(dsk, REG_CONTROL, CONTROL_SOFTWARE_RESET);
    wait(io);
    setreg(dsk, REG_CONTROL, CONTROL_ZERO);
}
typedef struct prdt {
	uint32_t buffer_phys;
	uint16_t transfer_size;
	uint16_t mark_end;
}__attribute__((packed)) prdt_t;
int is_slave(ata_dsk_t dsk) {
    if (dsk.disk == PRIMARY_SLAVE || dsk.disk == SECONDARY_SLAVE) {
        return true;
    }
}

void poll_ata(uint16_t io) {
    uint16_t s = inb(io+REG_CONTROL);
    while ((s & STATUS_BSY) || (s & STATUS_BSY))
    {
        s = inb(io+REG_CONTROL);
    }
    
}
#include <malloc.hpp>
int ata_rd(ata_dsk_t dsk, uint32_t sec, uint32_t *buf, uint32_t scount) {
    uint16_t io = get_io(dsk.disk);
    //log.info("0x%x %d 0x%x %d\n", io, sec, (uint32_t)buf, scount);
    outb(io+REG_CONTROL, 0x02);
    outb(io+REG_DRIVE, (0xE0 | (uint8_t)((sec >> 24 & 0x0F))));
    outb(io+REG_SEC_COUNT, scount);
    outb(io+REG_LBA_LO, sec & 0x000000ff);
    outb(io+REG_LBA_MID, (sec & 0x0000ff00) >> 8);
    outb(io+REG_LBA_HIGH, (sec & 0x00ff0000) >> 16);
    outb(io+REG_CONTROL, ATA_CMD_READ_PIO);
    poll_ata(io);
    uint16_t *b = (uint16_t *)buf;
    for (int i=0;i<256*scount;i++) {
        b[i] = inw(io);
    }
#if 1
    for (int i=0;i<512*scount;i++) {
        //printf("%c", buf[i]);
    }
    printf("\n");
#endif
    poll_ata(io);
    return 0;
}
int ata_wr(ata_dsk_t dsk, uint32_t sec, uint32_t *buf, uint32_t scount) {
    uint16_t io = get_io(dsk.disk);
    //log.info("0x%x %d 0x%x %d\n", io, sec, (uint32_t)buf, scount);
    outb(io+REG_CONTROL, 0x02);
    outb(io+REG_DRIVE, (0xE0 | (uint8_t)((sec >> 24 & 0x0F))));
    outb(io+REG_SEC_COUNT, scount);
    outb(io+REG_LBA_LO, sec & 0x000000ff);
    outb(io+REG_LBA_MID, (sec & 0x0000ff00) >> 8);
    outb(io+REG_LBA_HIGH, (sec & 0x00ff0000) >> 16);
    outb(io+REG_CONTROL, ATA_CMD_WRITE_PIO);
    poll_ata(io);
    uint16_t *b = (uint16_t *)buf;
    for (int i=0;i<256*scount;i++) {
        outw(io, b[i]);
    }
    
#if 1
    for (int i=0;i<512*scount;i++) {
        //printf("%c", buf[i]);
    }
    printf("\n");
#endif
    outb(io+REG_CONTROL, ATA_CMD_CACHE_FLUSH);
    poll_ata(io);
    return 0;
}

static void check_drives() {
    uint16_t pio = get_io(PRIMARY_MASTER);
    uint16_t sio = get_io(SECONDARY_MASTER);
    ata_identify_t *ident;
    uint16_t buf[256];
    outb(pio+REG_DRIVE, 0xA << 4); // select master drive
    outb(pio+REG_CONTROL, COMMAND_IDENTIFY);
    if (!(inb(pio+REG_CONTROL))) {
        log.info("Primary master not exists\n");
        goto ps;
    }
    log.info("Primary master exists\n");
    for(int i = 0; i < 256; i++) buf[i] = inw(pio);
    ident = (ata_identify_t *)&buf;
    log.info("Model: ");
    for (int i=0;i<40;i++) {
        printf("%c", ident->model[i]);
    }
    printf("\n");
    log.info("Sectors(LBA 28): %d\n", ident->sectors_28);
    log.info("Sectors(LBA 48): %d\n", ident->sectors_48);
ps:
    outb(pio+REG_DRIVE, (0xA + 1) << 4); // select slave drive
    outb(pio+REG_CONTROL, COMMAND_IDENTIFY);
    if (!(inb(pio+REG_CONTROL))) {
        log.info("Primary slave not exists\n");
        goto end;
    }
    log.info("Primary slave exists\n");
    for(int i = 0; i < 256; i++) inw(pio);
end:
    return;
}
namespace ATA
{
    void DetectDrives() {
        (void)dev_zero;
        log.info("Searching ATA controller...\n");
        dev = PCI::Get(0x8086, 0x7010, -1);
        if (dev.bits == 0) {
            log.info("No controller found\n");
            return;
        }
        log.info("Found controller, vendor: Intel\n");
        PCI::BusMasterEnable(dev);
        soft_rst(ata_disks[0]);
        soft_rst(ata_disks[2]);
        log.info("Software reset done.\n");
        setbothval(REG_LBA_LO, 0);
        setbothval(REG_LBA_MID, 0);
        setbothval(REG_LBA_HIGH, 0);
        
        uint32_t bar4 = PCI::Read(dev, PCI_BAR4);
        if (bar4 & 0x1) {
            bar4 = bar4 & 0xfffffffc;
        }
        bmr_cmd = bar4;
        bmr_stat = bar4 + 2;
        bmr_stat = bar4 + 4;
        check_drives();
    }    
} // namespace ATA


static void entry() {
    ATA::DetectDrives();
}

ADD_MODULE(entry, "ATA", NEED_PCI);