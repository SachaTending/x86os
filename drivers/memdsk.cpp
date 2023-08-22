#include <malloc.hpp>
extern "C" {
#include <fat_filelib.h>
}
#include <libc.hpp>
#define ATA_DISKS
#include <ata.hpp>

uint8 *dsk;

int rd(uint32 sec, uint8 *buf, uint32 cnt) {
    //printf("rd: %u %u\n", sec, cnt);
    //memcpy(buf, &dsk[sec*512], cnt*512);
    ata_rd(ata_disks[0], sec, (uint32_t *)buf, cnt);
    return 1;
}

int wr(uint32 sec, uint8 *buf, uint32 cnt) {
    //printf("wr: %u %u\n", sec, cnt);
#if 0
    printf("buf: \n");
    for (int i=0;i<cnt*512;i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif
    ata_wr(ata_disks[0], sec, (uint32_t *)buf, cnt);
    return 1;
}
void print_err(int err) {
    switch (err) {
        case FAT_INIT_ENDIAN_ERROR:
            printf("Endian error\n");
            break;
        case FAT_INIT_INVALID_SECTOR_SIZE:
            printf("Invalid sector size\n");
            break;
        case FAT_INIT_INVALID_SIGNATURE:
            printf("Invalid signature\n");
            break;
        case FAT_INIT_MEDIA_ACCESS_ERROR:
            printf("Media access error\n");
            break;
        case FAT_INIT_OK:
            printf("Ok\n");
            break;
        case FAT_INIT_STRUCT_PACKING:
            printf("Struct packing\n");
            break;
        case FAT_INIT_WRONG_FILESYS_TYPE:
            printf("Wrong filesystem type\n");
            break;
        case FAT_INIT_WRONG_PARTITION_TYPE:
            printf("Wrong partition type\n");
            break;
    }
}
void start() {
    fl_init();
    dsk = (uint8 *)malloc(512*50000);
    int s = fl_attach_media(rd, wr);
    void *fd;
    if (s != FAT_INIT_OK) {
        printf("err while initializating fatfs: ");
        print_err(s);
        return;
    }
    fl_listdirectory("/");
    printf("reading file...\n");
    fd = fl_fopen("/a", "r");
    fl_fseek(fd, 0, SEEK_END);
    int size = fl_ftell(fd);
    printf("size: %d\n", size);
    fl_fseek(fd, 0, SEEK_CUR);
    char *buf = (char *)malloc(size);
    memset(buf, 0, size);
    fl_fread((void *)buf, size, 1, fd);
    fl_fclose(fd);
    printf("%s", buf);
    printf("done\n");
}