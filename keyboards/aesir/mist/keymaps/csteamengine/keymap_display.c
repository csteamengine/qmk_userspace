// Dynamic keymap display for the Mist LCD. See keymap_display.h.
//
// Reproduces the hand-designed layer art: a top function row, three 2x3 blocks
// (left / center / right) with blank spacer boxes, and the 5 thumb keys below.
// Cell *positions* are fixed; the label in each cell is looked up live from the
// current layer via keymap_key_to_keycode(), so e.g. the center block is blank
// on the base layer and shows VOL/CODE on the FN0 layer automatically.
#include "keymap_display.h"

#include <stdio.h>
#include <string.h>

#include "quantum.h"
#include "qp.h"
#include "rgb_matrix.h"
#include "fonts/font_proggy_tiny.qff.h"
#include "fonts/norse20.qff.h"
#include "graphics/hermod-logo.qgf.h"

// Status-line helpers defined in keymap.c (reused so we don't duplicate the
// RGB mode name switch / layer name table).
const char *current_rgb_mode(void);
const char *current_layer_name(void);

// Design grid the art is authored in; scaled to the real panel.
#define ART_W 128
#define ART_H 96

// The panel's drawing space (matches init_lcd's qp_rect(lcd,0,0,320,240,...)).
// qp_get_width/height report the rotated 240x320, but drawing happens in 320x240.
#define KD_W 320
#define KD_H 240

// Marker in .row for a purely decorative empty box (no key behind it).
#define KD_BLANK 0xFF

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t ax, ay, aw, ah; // rect in the ART_W x ART_H design grid
} artkey_t;

// clang-format off
static const artkey_t keys[] = {
    // ---- top function row in three groups of 4 (screenshots | media | passwords) ----
    // ay = 15 leaves a line of room above for the caps/layer/RGB status text, and
    // brings the row closer to the middle blocks.
    {0, 1,   3,15, 8, 11}, {0, 2,  12,15, 8, 11}, {0, 3,  21,15, 8, 11}, {0, 4,  30,15, 8, 11},
    {0, 5,  46,15, 8, 11}, // group 2: gap on base (KC_NO), RGB mode-prev on FN0
    {0, 6,  55,15, 8, 11}, {0, 7,  64,15, 8, 11}, {0, 8,  73,15, 8, 11},
    {0, 9,  89,15, 8, 11}, {0, 10, 98,15, 8, 11}, {0, 11,107,15, 8, 11}, {0, 12,116,15, 8, 11},

    // ---- left block (EMOJI / TODO / NOTE / APP) ----
    {KD_BLANK, 0,  4, 34, 11, 12}, {KD_BLANK, 0, 15, 34, 11, 12}, {5, 2,  26, 34, 11, 12},
    {6, 0,         4, 49, 11, 12}, {6, 1,        15, 49, 11, 12}, {6, 2,  26, 49, 11, 12},

    // ---- center block: arrows only (UP top, LT/DN/RT below); VOL/CODE on FN0 ----
    {5, 9,        58, 34, 11, 12},
    {6, 8,        47, 49, 11, 12}, {6, 9,        58, 49, 11, 12}, {6, 10,       69, 49, 11, 12},

    // ---- right block: 2x2 (REFACTOR, blank / HOME, END) ----
    {5, 11,     101, 34, 11, 12}, {KD_BLANK, 0, 112, 34, 11, 12},
    {6, 11,     101, 49, 11, 12}, {6, 12,     112, 49, 11, 12},

    // ---- thumb cluster (LAYER dropped slightly, like the art) ----
    {5, 3, 31, 66, 11, 12}, {5, 4, 44, 66, 11, 12}, {5, 5, 57, 72, 11, 12}, {5, 6, 70, 66, 11, 12}, {5, 7, 83, 66, 11, 12},
};
// clang-format on

#define NKEYS (sizeof(keys) / sizeof(keys[0]))

static uint16_t cellx[NKEYS], celly[NKEYS], cellw[NKEYS], cellh[NKEYS];
static uint16_t cache_kc[NKEYS];
static uint8_t  cache_layer[NKEYS];
static bool     laid_out = false;

static void compute_layout(void) {
    for (size_t i = 0; i < NKEYS; i++) {
        cellx[i] = (uint16_t)((uint32_t)keys[i].ax * KD_W / ART_W);
        celly[i] = (uint16_t)((uint32_t)keys[i].ay * KD_H / ART_H);
        cellw[i] = (uint16_t)((uint32_t)keys[i].aw * KD_W / ART_W);
        cellh[i] = (uint16_t)((uint32_t)keys[i].ah * KD_H / ART_H);
    }
    laid_out = true;
}

static void draw_text_centered(painter_device_t display, painter_font_handle_t font, uint16_t x0, uint16_t w, int16_t y, const char *str) {
    int16_t tw = qp_textwidth(font, str);
    int16_t tx = x0 + ((int16_t)w - tw) / 2;
    if (tx < (int16_t)x0 + 1) tx = x0 + 1;
    qp_drawtext(display, tx, y, font, str);
}

static void draw_cell(painter_device_t display, painter_font_handle_t font, size_t i, uint16_t kc, bool blank) {
    const uint16_t x0 = cellx[i];
    const uint16_t y0 = celly[i];
    const uint16_t w  = cellw[i];
    const uint16_t h  = cellh[i];
    const uint16_t x1 = x0 + w - 1;
    const uint16_t y1 = y0 + h - 1;

    qp_rect(display, x0, y0, x1, y1, HSV_BLACK, true); // always clear first

    // Every cell gets an outlined box (blanks/KC_NO/KC_TRNS show as empty boxes).
    qp_rect(display, x0, y0, x1, y1, HSV_WHITE, false);

    if (blank || font == NULL) {
        return;
    }

    const char *label = keycode_label(kc);
    if (label == NULL || label[0] == '\0') {
        return; // transparent / unlabeled key: outline only
    }

    char buf[24];
    strncpy(buf, label, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char       *space = strchr(buf, ' ');
    const char *line1 = buf;
    const char *line2 = NULL;
    if (space != NULL) {
        *space = '\0';
        line2  = space + 1;
    }

    const uint8_t  lh    = font->line_height;
    const uint16_t total = line2 ? (uint16_t)(lh * 2) : lh;
    int16_t        ty    = y0 + ((int16_t)h - (int16_t)total) / 2;
    if (ty < (int16_t)y0 + 1) ty = y0 + 1;

    draw_text_centered(display, font, x0, w, ty, line1);
    if (line2 != NULL) {
        draw_text_centered(display, font, x0, w, ty + lh, line2);
    }
}

// Status line across the very top: caps state, active layer, RGB mode. Redrawn
// only when one of them changes. Lives above the top key row (which starts at
// art y=15 -> ~37px).
#define KD_STATUS_CLEAR 36

static uint8_t st_caps  = 0xFF;
static uint8_t st_layer = 0xFF;
static uint8_t st_rgb   = 0xFF;

static void draw_status(painter_device_t display) {
    const bool    caps  = host_keyboard_led_state().caps_lock;
    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    const uint8_t rgb   = rgb_matrix_get_mode();

    if (caps == st_caps && layer == st_layer && rgb == st_rgb) {
        return;
    }
    st_caps  = caps;
    st_layer = layer;
    st_rgb   = rgb;

    const painter_font_handle_t font = qp_load_font_mem(font_norse20);
    if (font == NULL) {
        return;
    }

    qp_rect(display, 0, 0, KD_W - 1, KD_STATUS_CLEAR - 1, HSV_BLACK, true);

    char buf[24];
    snprintf(buf, sizeof(buf), "Caps:%s", caps ? "On" : "Off");
    qp_drawtext(display, 2, 2, font, buf);

    const char *lname = current_layer_name();
    qp_drawtext(display, (KD_W - qp_textwidth(font, lname)) / 2, 2, font, lname);

    const char *rmode = current_rgb_mode();
    qp_drawtext(display, KD_W - qp_textwidth(font, rmode) - 2, 2, font, rmode);

    qp_close_font(font);
}

// Static logo in the empty bottom-right corner. Drawn once per (re)init.
static void draw_logo(painter_device_t display) {
    const painter_image_handle_t logo = qp_load_image_mem(gfx_hermod_logo);
    if (logo == NULL) {
        return;
    }
    qp_drawimage(display, KD_W - logo->width - 14, KD_H - logo->height - 12, logo);
    qp_close_image(logo);
}

// "MIST" wordmark in the empty bottom-left corner. Drawn once per (re)init.
static void draw_brand(painter_device_t display) {
    const painter_font_handle_t font = qp_load_font_mem(font_norse20);
    if (font == NULL) {
        return;
    }
    qp_drawtext(display, 3, KD_H - font->line_height - 4, font, "MIST");
    qp_close_font(font);
}

void keymap_display_init(painter_device_t display) {
    qp_rect(display, 0, 0, KD_W - 1, KD_H - 1, HSV_BLACK, true);
    compute_layout();
    draw_logo(display);
    draw_brand(display);

    st_caps  = 0xFF;
    st_layer = 0xFF;
    st_rgb   = 0xFF;
    for (size_t i = 0; i < NKEYS; i++) {
        cache_kc[i]    = 0xFFFF;
        cache_layer[i] = 0xFF;
    }
    keymap_display_render(display);
}

void keymap_display_render(painter_device_t display) {
    if (!laid_out) {
        compute_layout();
    }

    draw_status(display);

    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    const painter_font_handle_t font = qp_load_font_mem(font_ProggyTiny15);
    for (size_t i = 0; i < NKEYS; i++) {
        const bool blank = (keys[i].row == KD_BLANK);

        uint16_t kc;
        uint8_t  lyr;
        if (blank) {
            kc  = 0xFFFE; // constant signature for decorative boxes
            lyr = 0;
        } else {
            const keypos_t pos = {.row = keys[i].row, .col = keys[i].col};
            kc  = keymap_key_to_keycode(layer, pos);
            lyr = layer;
        }

        if (kc == cache_kc[i] && lyr == cache_layer[i]) {
            continue;
        }
        cache_kc[i]    = kc;
        cache_layer[i] = lyr;
        draw_cell(display, font, i, kc, blank);
    }
    if (font != NULL) {
        qp_close_font(font);
    }
}
