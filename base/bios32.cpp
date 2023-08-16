#include <libc.hpp>
#include <logging.hpp>
#include <bios32.hpp>

static Logging log("BIOS32");

namespace BIOS32
{
    void Init() {
        log.info("Searching for bios32...\n");
        uint32_t mem = 0xE0000;
        for (mem=0xE0000;mem<0xFFFFF;mem++) {
            //log.info("0x%x\n", mem);
            if (*((uint32_t*)mem) == 0x5F32335F) {
                log.info("BIOS32 Found at addr 0x%x.\n", mem);
            }
        }
    }
} // namespace BIOS32
