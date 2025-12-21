#include "arch/i686/disp.h"
#include <util/font8x8.h>
#include <string.h>
#include <stdbool.h>

/* =========================
   Driver state
   ========================= */
static TAG_FB* drv_fb = 0;
static uint32_t drv_bytes_per_pixel = 0;
static uint32_t drv_pitch = 0;
static uint32_t drv_scale = 1;

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

/* Colors */
static uint32_t fg_color = 0;

/* =========================
   ANSI parser state
   ========================= */
static bool ansi_esc = false;
static bool ansi_csi = false;
static int ansi_params[4];
static int ansi_pcount = 0;
static int ansi_value = 0;

/* =========================
   Init / clear
   ========================= */
void i686_DISP_Initialize(TAG_FB* fb) {
    drv_fb = fb;
    if (!fb) return;
    drv_bytes_per_pixel = fb->bpp / 8;
    drv_pitch = fb->pitch ? fb->pitch : (fb->width * drv_bytes_per_pixel);
    fg_color = 0xFFFFFFFF;
}

void i686_DISP_Clear() {
    if (!drv_fb) return;

    memset(drv_fb->fb, 0, drv_pitch * drv_fb->height);
    cursor_x = 0;
    cursor_y = 0;
}

void i686_DISP_SetScale(uint32_t scale) {
    drv_scale = scale ? scale : 1;
}

/* =========================
   Pixel helpers
   ========================= */
static void drv_write_pixel_raw(uint32_t x, uint32_t y, uint32_t pixel) {
    if (!drv_fb) return;
    if (x >= drv_fb->width || y >= drv_fb->height) return;

    uint8_t* p = drv_fb->fb + y * drv_pitch + x * drv_bytes_per_pixel;
    for (uint32_t i = 0; i < drv_bytes_per_pixel; i++)
        p[i] = (pixel >> (8 * i)) & 0xFF;
}

static uint32_t drv_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t pix = 0;
    pix |= (r >> (8 - drv_fb->red_mask)) << drv_fb->red_position;
    pix |= (g >> (8 - drv_fb->green_mask)) << drv_fb->green_position;
    pix |= (b >> (8 - drv_fb->blue_mask)) << drv_fb->blue_position;
    return pix;
}

/* =========================
   Glyph rendering
   ========================= */
static void drv_put_glyph(char c, uint32_t x, uint32_t y) {
    uint32_t bg = drv_rgb(0,0,0);
    uint32_t cw = 8 * drv_scale;
    uint32_t ch = 8 * drv_scale;

    for (uint32_t ry = 0; ry < ch; ry++)
        for (uint32_t rx = 0; rx < cw; rx++)
            drv_write_pixel_raw(x + rx, y + ry, bg);

    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;

    for (uint32_t row = 0; row < 8; row++) {
        unsigned char bits = font8x8_basic[uc][row];
        for (uint32_t col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                for (uint32_t sy = 0; sy < drv_scale; sy++)
                    for (uint32_t sx = 0; sx < drv_scale; sx++)
                        drv_write_pixel_raw(
                            x + col * drv_scale + sx,
                            y + row * drv_scale + sy,
                            fg_color
                        );
            }
        }
    }
}

/* =========================
   ANSI execution
   ========================= */
static void ansi_execute(char cmd) {
    uint32_t cw = 8 * drv_scale;
    uint32_t ch = 8 * drv_scale;

    switch (cmd) {
        case 'A': cursor_y -= ch; break;
        case 'B': cursor_y += ch; break;
        case 'C': cursor_x += cw; break;
        case 'D': cursor_x -= cw; break;

        case 'H': {
            uint32_t row = ansi_params[0] ? ansi_params[0] - 1 : 0;
            uint32_t col = ansi_params[1] ? ansi_params[1] - 1 : 0;
            cursor_x = col * cw;
            cursor_y = row * ch;
            break;
        }

        case 'J':
            if (ansi_params[0] == 2) i686_DISP_Clear();
            break;

        case 'K':
            for (uint32_t x = cursor_x; x < drv_fb->width; x += cw)
                drv_put_glyph(' ', x, cursor_y);
            break;

        case 'm':
            if (ansi_params[0] == 0)
                fg_color = drv_rgb(255,255,255);
            else if (ansi_params[0] == 31)
                fg_color = drv_rgb(255,0,0);
            else if (ansi_params[0] == 32)
                fg_color = drv_rgb(0,255,0);
            else if (ansi_params[0] == 33)
                fg_color = drv_rgb(255,255,0);
            else if (ansi_params[0] == 34)
                fg_color = drv_rgb(0,0,255);
            break;
    }
}

/* =========================
   Character output
   ========================= */
void i686_DISP_PutChar(char c) {
    if (!drv_fb) return;

    uint32_t cw = 8 * drv_scale;
    uint32_t ch = 8 * drv_scale;

    /* ANSI parser */
    if (ansi_esc) {
        if (c == '[') {
            ansi_esc = false;
            ansi_csi = true;
            ansi_pcount = 0;
            ansi_value = 0;
            memset(ansi_params, 0, sizeof(ansi_params));
            return;
        }
        ansi_esc = false;
        return;
    }

    if (ansi_csi) {
        if (c >= '0' && c <= '9') {
            ansi_value = ansi_value * 10 + (c - '0');
            return;
        }
        if (c == ';') {
            ansi_params[ansi_pcount++] = ansi_value;
            ansi_value = 0;
            return;
        }
        ansi_params[ansi_pcount++] = ansi_value;
        ansi_execute(c);
        ansi_csi = false;
        return;
    }

    if ((unsigned char)c == 0x1B) {
        ansi_esc = true;
        return;
    }

    /* Control chars */
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += ch;
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    if (c == '\b') {
        drv_put_glyph(' ', cursor_x, cursor_y);
        if (cursor_x >= cw) cursor_x -= cw;
        drv_put_glyph(' ', cursor_x, cursor_y);
        return;
    }

    /* Printable */
    drv_put_glyph(c, cursor_x, cursor_y);
    cursor_x += cw;

    if (cursor_x + cw > drv_fb->width) {
        cursor_x = 0;
        cursor_y += ch;
    }
}

void i686_DISP_PutString(const char* s) {
    while (*s) i686_DISP_PutChar(*s++);
}

/* =========================
   Cursor blinking
   ========================= */
static uint32_t cursor_counter = 0;
static bool cursor_visible = false;

void i686_DISP_ToggleCursor(void) {
    if (!drv_fb) return;

    cursor_counter++;
    if (cursor_counter < 500) return;
    cursor_counter = 0;

    uint32_t white = drv_rgb(255,255,255);
    uint32_t black = drv_rgb(0,0,0);
    uint32_t cw = 8 * drv_scale;
    uint32_t ch = 8 * drv_scale;

    if (cursor_visible) {
        for (uint32_t y = 0; y < ch; y++)
            for (uint32_t x = 0; x < cw; x++)
                drv_write_pixel_raw(cursor_x + x, cursor_y + y, black);
        cursor_visible = false;
    } else {
        for (uint32_t y = 0; y < ch; y++)
            for (uint32_t x = 0; x < cw; x++)
                drv_write_pixel_raw(cursor_x + x, cursor_y + y, white);
        cursor_visible = true;
    }
}
