#include <multiboot.h>
#include <logging.hpp>
#include <malloc.hpp>
#include <libc.hpp>
#include <stddef.h>

static Logging log("Memory");

extern multiboot_info_t *mbi;
extern int kernel_start;
const char * mmap_type_to_string(multiboot_uint32_t type);

static int alloc_id;

struct alloc_struct
{
    bool allocated;
    int id;
    int size;
    char magic[4];
    alloc_struct *prev;
    alloc_struct *next;
};

multiboot_mmap_entry big_entry;

void pmm_init() {
    // Parse memmap
    log.info("Parsing memmap...\n");
    multiboot_memory_map_t *mmap;
	for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
           (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
           mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                    + mmap->size + sizeof (mmap->size))) {
        log.info (" size = 0x%x, base_addr = 0x%x%08x,"
                " length = 0x%x%08x, type = %s\n",
                (unsigned) mmap->size,
                (unsigned) (mmap->addr >> 32),
                (unsigned) (mmap->addr & 0xffffffff),
                (unsigned) (mmap->len >> 32),
                (unsigned) (mmap->len & 0xffffffff),
                mmap_type_to_string(mmap->type));
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mmap->size > big_entry.size && mmap->addr != kernel_start) {
                big_entry.addr = mmap->addr;
                big_entry.len = mmap->len;
                big_entry.size = mmap->size;
                big_entry.type = mmap->type;
            }
        }
    }
    log.info("Big entry: addr=0x%x len=%d\n", big_entry.addr, big_entry.len);
    alloc_struct *d = (alloc_struct *)big_entry.addr;
    d->allocated = false;
    d->next = d;
    d->prev = d;
    d->id = 0;
    d->size = 0;
    strcpy((char *)&d->magic, "BRUH");
    alloc_id++;
}

bool check_signature(alloc_struct *alloc) {
    return !strcmp((const char *)&alloc->magic, (char *)"BRUH");
}

void set_signature(alloc_struct *alloc) {
    strcpy((char *)&alloc->magic, "BRUH");
}

void debug_null(const char *m, ...) {

}

#define debug debug_null


extern "C" {
    void * malloc(size_t size) {
        alloc_struct *d = (alloc_struct *)(big_entry.addr + sizeof(alloc_struct));
        alloc_struct *prev;
        debug("Searching block size: %u\n", size);
        while ((int)d < (big_entry.len + big_entry.addr)) {
            debug("Addr: 0x%x\n", d);
            if (check_signature(d)) {
                // This is allocated block
                debug("Found block\n");
                if (d->allocated == false && d->size >= size) {
                    // Found free block, set allocated to true and return it
                    debug("Block is unallocated");
                    d->allocated = true;
                    return d += sizeof(alloc_struct);
                } else {
                    debug("Block allocated.\n");
                    prev = d;
                    d += d->size + sizeof(alloc_struct);
                }
            } else {
                // Found unallocated space
                if (((int)d + sizeof(alloc_struct) + size) > (big_entry.addr + big_entry.len)) {
                    // bruh
                    return (void *)-1;
                } else {
                    // Allocate space
                    d->allocated = true;
                    d->prev = prev;
                    prev->next = d;
                    set_signature(d);
                    d->size = size;
                    d->id = alloc_id;
                    alloc_id++;
                    return (void *)d + sizeof(alloc_struct);
                }
            }
        }
    }
    void free(void *mem) {
        alloc_struct *d= (alloc_struct *)(mem - sizeof(alloc_struct));
        if (check_signature(d)) {
            if (d->allocated) {
                d->allocated = false;
            }
        }
    }
    void flanterm_free(void *mem, size_t bruh) {
        free(mem);
    }
}