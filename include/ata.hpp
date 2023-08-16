#pragma once
#include <stdint.h>
namespace ATA
{
    void DetectDrives();   
} // namespace ATA
enum ATA_DISK {
    PRIMARY_MASTER,
    PRIMARY_SLAVE,

    SECONDARY_MASTER,
    SECONDARY_SLAVE
};
typedef struct 
{
    ATA_DISK disk;
    char name[40];
} ata_dsk_t;
#ifdef ATA_DISKS
extern ata_dsk_t ata_disks[4];
#endif
int ata_rd(ata_dsk_t dsk, uint32_t sec, uint32_t *buf, uint32_t scount);
// Registers

#define REG_DATA 0x0
#define REG_ERROR 0x1
#define REG_SEC_COUNT 0x2
#define REG_LBA_LO 0x3
#define REG_LBA_MID 0x4
#define REG_LBA_HIGH 0x5
#define REG_DRIVE 0x6
#define REG_CONTROL 0x7
#define REG_ALT_STATUS 0x206

// Controll register
#define CONTROL_STOP_INTERRUPT 0x2
#define CONTROL_SOFTWARE_RESET 0x4
#define CONTROL_HIGH_ORDER_BYTE 0x80
#define CONTROL_ZERO 0x00

// Command reg
#define COMMAND_IDENTIFY 0xEC
#define COMMAND_DMA_READ 0xC8
#define ATA_CMD_READ_PIO 0x20

// Status reg
#define STATUS_ERR 0x0
#define STATUS_DRQ 0x8
#define STATUS_SRV 0x10
#define STATUS_DF  0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80