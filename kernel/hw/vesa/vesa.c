#include "vesa.h"
#include "paging/paging.h"
static uint32_t *vesa_framebuffer;
static uint16_t vesa_pitch;

void vesa_init(void) {
    vesa_framebuffer = *(uint32_t**)VGA_VESA_FB_ADDR;
    vesa_pitch = *(uint16_t*)0x7e10 / 4;
    const page_directory *kpd = paging_get_kernel_pd();
    uint32_t fb_size = VESA_WIDTH * VESA_HEIGHT * VESA_COLOR_CHANNELS;
    for (uint32_t offset = 0; offset < fb_size; offset+=4096) {
        paging_map((page_directory*)kpd, (uint32_t)vesa_framebuffer + offset, (uint32_t)vesa_framebuffer + offset, PAGING_PD_ENTRY_FLAGS_KERNEL_ONLY);
    }
}

void vesa_clear(uint32_t color) {
    for (uint32_t y = 0; y < VESA_HEIGHT; y++) {
        uint32_t *line = vesa_framebuffer + y * vesa_pitch;
        for (uint32_t x = 0; x < VESA_WIDTH; ++x) {
            *line++ = color;
        }
    }
}

uint32_t *vesa_get_fb(void) {
    return vesa_framebuffer;
}

uint16_t vesa_get_pitch(void) {
    return vesa_pitch;
}
