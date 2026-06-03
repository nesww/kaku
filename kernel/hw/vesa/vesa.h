#ifndef VESA_H
#define VESA_H

#define VGA_VESA_FB_ADDR    0x600
#define VESA_WIDTH          1024
#define VESA_HEIGHT         768
#define VESA_COLOR_CHANNELS 4

#include <stdint.h>

void vesa_init(void);
uint32_t *vesa_get_fb(void);
uint16_t vesa_get_pitch(void);
void vesa_clear(uint32_t color);
void vesa_set_pixel(uint32_t x, uint32_t y, uint32_t color);
#endif //VESA_H
