#include <logging.hpp>
#include <acpi.hpp>
#include <libc.hpp>

static Logging log("ACPI");

struct RSDPDescriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

RSDPDescriptor *rsdp;
ACPISDTHeader *rsdt;

uint8_t lapic_ids[256] = {0};  // CPU core Local APIC IDs
uint8_t numcore = 0;           // number of cores detected
uint64_t lapic_ptr = 0;        // pointer to the Local APIC MMIO registers
uint64_t ioapic_ptr = 0;       // pointer to the IO APIC MMIO registers

void detect_cores(uint8_t *rsdt) {
    uint8_t *ptr, *ptr2;
    uint32_t len;

    // iterate on ACPI table pointers
    for (len = *((uint32_t*)(rsdt + 4)), ptr2 = rsdt + 36; ptr2 < rsdt + len; ptr2 += rsdt[0] == 'X' ? 8 : 4) {
        ptr = (uint8_t*)(uintptr_t)(rsdt[0] == 'X' ? *((uint64_t*)ptr2) : *((uint32_t*)ptr2));
        if (!memcmp(ptr, (uint8_t *)"APIC", 4)) {
            // found MADT
            lapic_ptr = (uint64_t)(*((uint32_t*)(ptr + 0x24)));
            ptr2 = ptr + *((uint32_t*)(ptr + 4));
            break;
            // iterate on variable length records
            for (ptr += 44; ptr < ptr2; ptr += ptr[1]) {
                if (ptr[1] > 0) {
                    log.info("0x%x 0x%x %d\n", ptr, ptr2, ptr[0]);
                }
                switch (ptr[0]) {
                    case 0: if (ptr[4] & 1) lapic_ids[numcore++] = ptr[3]; break; // found Processor Local APIC
                    case 1: ioapic_ptr = (uint64_t)*((uint32_t*)(ptr + 4)); break;  // found IOAPIC
                    case 5: lapic_ptr = *((uint64_t*)(ptr + 4)); break;             // found 64 bit LAPIC
                }
            }
            break;
        }
    }
}

static bool isChecksumValidRSDP(uint8_t* rsdp) {
    uint8_t sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += rsdp[i];
    }
    return (sum == 0);
}

static bool is_rsdp(int *mem) {
    char *mem2 = (char *)mem;
    if (mem2[1] == 'R') {
        mem2++;
    }
    if (mem2[0] == 'R' && mem2[1] == 'S' && mem2[2] == 'D' && mem2[3] == ' ') {
        if (mem2[4] == 'P' && mem2[5] == 'T' && mem2[6] == 'R' && mem2[7] == ' ') {
            return isChecksumValidRSDP((uint8_t *)mem2);
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool ACPI::FindRSDP() {
    int *mem = (int *)0x000E0000;
    while ((int)mem < 0x000FFFFF) {
        if (is_rsdp(mem)) {
            log.info("Found RSDP at 0x%08x\n", (int)mem);
            rsdp = (RSDPDescriptor *)mem;
            return true;
        }
        mem+=4;
        //log.info("0x%08x %s\n", mem, ((RSDPDescriptor *)mem)->Signature); // debug function
    }
    log.info("RSDP Not found.\n");
    return false;
}

bool g_ACER = false;

void ACPI::Init() {
    log.info("Searching RSDP...\n");
    if (ACPI::FindRSDP()) {
        log.info("OEMID: ");
        for (int i=0;i<sizeof(rsdp->OEMID);i++) {
            printf("%c", rsdp->OEMID[i]);
        } printf("\n");
        if (!strcmp("ACR", (char *)&rsdt->OEMID)) 
        {
            g_ACER = true;
            log.info("Detected OEM: Acer\n");
        }
        log.info("Revision: %u\n", rsdp->Revision);
        rsdt = (ACPISDTHeader *)rsdp->RsdtAddress;
        log.info("OEMID(RSDT): ");
        for (int i=0;i<6;i++) printf("%c", rsdt->OEMID[i]);
        printf("\n");
        detect_cores((uint8_t *)rsdt);
    }
}