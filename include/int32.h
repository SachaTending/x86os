#pragma once

// define our structure
typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

// tell compiler our int32 function is external
#ifdef __cplusplus
extern "C" {
#endif
extern void int32(unsigned char intnum, regs16_t *regs);
#ifdef __cplusplus
}
#endif

#define seg(x) ((uint16_t)(((int)x & 0xffff0) >> 4)) // This is from limine src.
#define off(x) ((uint16_t)(((int)x & 0x0000f) >> 0))
#define desegment(seg, off) (((uint32_t)(seg) << 4) + (uint32_t)(off))