#include <malloc.hpp>
extern "C" {
#include <fat_filelib.h>
}
#include <libc.hpp>
#define ATA_DISKS
#include <ata.hpp>

uint8 *dsk;

int rd(uint32 sec, uint8 *buf, uint32 cnt) {
    printf("rd: %u %u\n", sec, cnt);
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
    for (int i=0;i<512*cnt;i++) {
        dsk[i+(sec*512)] = buf[i];
    }
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
    if (s != FAT_INIT_OK) {
        printf("err while initializating fatfs: ");
        print_err(s);
        return;
    }
    printf("creating file...\n");
    void *fd = 0;
    //void *fd = fl_fopen("/a", "w");
    //fl_fwrite("hello", 6, 1, fd);
    //fl_fclose(fd);
    printf("done\n");
    fl_listdirectory("/");
    printf("reading file...\n");
    fd = fl_fopen("/a", "r");
    char buf[10];
    fl_fread((void *)&buf, 6, 1, fd);
    fl_fclose(fd);
    printf("%s\n", buf);
    printf("done\n");
}