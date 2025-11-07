#include "arch/i686/disp.h"
#include <util/font8x8.h>
#include <memory.h>

// Simple driver state
static TAG_FB* drv_fb = 0;
static uint32_t drv_bytes_per_pixel = 0;
static uint32_t drv_pitch = 0;
static uint32_t drv_scale = 1; // <--- New: scale modifier (default 1x)

// Cursor position for putc/puts
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

void i686_DISP_Initialize(TAG_FB* fb) {
    drv_fb = fb;
    if (!fb) return;
    drv_bytes_per_pixel = fb->bpp / 8;
    drv_pitch = fb->pitch ? fb->pitch : (fb->width * drv_bytes_per_pixel);
}

// Set the scale factor (must be >= 1)
void i686_DISP_SetScale(uint32_t scale) {
    if (scale <= 0) scale = 1;
    drv_scale = scale;
}

static void drv_write_pixel_raw(uint32_t x, uint32_t y, uint32_t pixel) {
    if (!drv_fb) return;
    if (x >= drv_fb->width || y >= drv_fb->height) return;

    uint8_t* base = drv_fb->fb;
    uint8_t* p = base + y * drv_pitch + x * drv_bytes_per_pixel;
    for (uint32_t i = 0; i < drv_bytes_per_pixel; ++i) {
        p[i] = (uint8_t)((pixel >> (8 * i)) & 0xFF);
    }
}

void i686_DISP_PutPixel(uint32_t x, uint32_t y, uint32_t pixel) {
    drv_write_pixel_raw(x, y, pixel);
}

static uint32_t drv_rgb_to_pixel(uint8_t r, uint8_t g, uint8_t b) {
    if (!drv_fb) return 0;
    if (drv_fb->red_mask == 0 && drv_fb->green_mask == 0 && drv_fb->blue_mask == 0) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }
    uint32_t pix = 0;
    if (drv_fb->red_mask) {
        uint32_t rs = (uint32_t)r >> (8 - drv_fb->red_mask);
        pix |= (rs << drv_fb->red_position);
    }
    if (drv_fb->green_mask) {
        uint32_t gs = (uint32_t)g >> (8 - drv_fb->green_mask);
        pix |= (gs << drv_fb->green_position);
    }
    if (drv_fb->blue_mask) {
        uint32_t bs = (uint32_t)b >> (8 - drv_fb->blue_mask);
        pix |= (bs << drv_fb->blue_position);
    }
    return pix;
}

static void drv_put_glyph(char c, uint32_t x, uint32_t y, uint32_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;
    for (uint32_t row = 0; row < 8; ++row) {
        unsigned char bits = font8x8_basic[uc][row];
        for (uint32_t col = 0; col < 8; ++col) {
            if (bits & (1 << col)) {
                // Apply scaling
                for (uint32_t sy = 0; sy < drv_scale; ++sy) {
                    for (uint32_t sx = 0; sx < drv_scale; ++sx) {
                        drv_write_pixel_raw(
                            x + col * drv_scale + sx,
                            y + row * drv_scale + sy,
                            color
                        );
                    }
                }
            }
        }
    }
}

void i686_DISP_PutChar(char c) {
    if (!drv_fb) return;
    uint32_t white = drv_rgb_to_pixel(255,255,255);
    uint32_t black = drv_rgb_to_pixel(0,0,0);

    uint32_t char_width = 8 * drv_scale;
    uint32_t char_height = 8 * drv_scale;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += char_height;
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    if (c == '\b') {
        if (cursor_x >= char_width) cursor_x -= char_width;
        drv_put_glyph(' ', cursor_x, cursor_y, black);
        return;
    }

    drv_put_glyph(c, cursor_x, cursor_y, white);
    cursor_x += char_width;
    if (cursor_x + char_width > drv_fb->width) {
        cursor_x = 0;
        cursor_y += char_height;
    }
}

void i686_DISP_PutString(const char* s) {
    for (const char* p = s; *p; ++p) i686_DISP_PutChar(*p);
}
