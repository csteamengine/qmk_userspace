// Dynamic keymap display for the Mist LCD.
//
// Renders a grid of the board's "special" keys on the ili9341 using the
// elpekenin/ui community module. Each cell reads the *live* keycode for the
// current layer via keymap_key_to_keycode(), so the labels follow your real
// mapping (and switch automatically when you change layers or remap a key).
#pragma once

#include "qp.h"

// Build the node tree + paint the initial frame. Call once, after the LCD and
// fonts are initialised.
void keymap_display_init(painter_device_t display);

// Re-render changed cells. Call periodically (housekeeping task).
void keymap_display_render(painter_device_t display);

// Maps a keycode to a short, human-friendly label (e.g. "PW1", "REC SCREEN").
// A single space splits the label across two lines. Returns "" for keys that
// should render as an empty box (KC_NO / KC_TRNS). Defined in keymap.c so it
// can see the tap-dance enum and custom keycodes.
const char *keycode_label(uint16_t keycode);
