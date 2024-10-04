// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "quantum.h"
// #include "secrets.h"
#include "csteamengine.h"
#include "lib/layer_status/layer_status.h"
// #include "lib/bongocat/bongocat.h"
#include <qp.h>

painter_device_t lcd;

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * This is a standard version of this same keyboard. I added optional 2u keys for 0, + and Enter on the numpad
     * For my personal board, I will be splitting them up to have more macros, but if you want a more standard layout
     * You can install this one. The one downside with this, is you won't have arrow keys on the main layer anymore.
     *       ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
     *       │M0 │M1 │M2 │M3 │M4 │M5 │M6 │M7 │M8 │M9 │M10│M11│M12│M13│M14│
     *       └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
     * ┌───┐ ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐ ┌───┬───┬───┬───┐
     * │M17│ │Esc│F1 │F2 │F3 │F4 │F5 │F6 │F7 │F8 │F9 │F10│F11│F12│F13│M15│ │Del│End│Hom│M16│
     * ├───┤ ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┴───┤ ┌───┬───┬───┬───┐
     * │M18│ │ ` │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│ │ + │ - │ * │ / │
     * ├───┤ ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┼───┤
     * │M19│ │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │ │ 7 │ 8 │ 9 │   │ <-- KC_NO filler
     * ├───┤ ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ ├───┼───┼───┤ + │
     * │M20│ │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │ │ 4 │ 5 │ 6 │   │
     * ├───┤ ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤ ├───┼───┼───┼───┤
     * │M21│ │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │    Shift │ │ 1 │ 2 │ 3 │   │ <-- KC_NO filler
     * ├───┤ ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ├───┴───┼───┤Ent│
     * │M22│ │Ctrl│GUI │Alt │                        │ Alt│ GUI│Menu│Ctrl│ │   0   │ . │   │
     * └───┘ └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘ └───────┴───┴───┘
     *                                                                       ^-- KC_NO filler
     */
    /*  Row:    0         1        2        3         4      */
    [_BASE] = LAYOUT(
                KC_LSFT, SELECT_WORD, TO(_FN0)
            ),

    /*  Row:    0        1        2        3        4       */
    [_FN0] = LAYOUT(

                TO(_BASE), CW_TOGG, TO(_FN1)
            ),

    /*  Row:    0        1        2        3        4       */
    [_FN1] = LAYOUT(
                TO(_FN0), SC_LSPO, TO(_FN2)
                ),

    /*  Row:    0        1        2        3        4        */
    [_FN2] = LAYOUT(
                TO(_FN1), SC_RSPC, TO(_BASE)
            ),
};

// clang-format on
#ifdef OLED_ENABLE
bool oled_task_keymap(void) {
    render_layer_status();
    // render_bongocat();

    return true;
}
#endif

bool process_record_keymap (uint16_t keycode, keyrecord_t *record) {

  return true;
}


void keyboard_post_init_user(void) {
    // Let the LCD get some power...
    wait_ms(200);

    // Initialize the LCD
    lcd = qp_ili9341_make_spi_device(240, 320, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 0);
    qp_init(lcd, QP_ROTATION_0);  // Try different rotations

    backlight_enable();
    backlight_level(1);

    // Turn on the LCD and clear the display
    qp_power(lcd, true);
    qp_rect(lcd, 0, 0, 239, 319, 0,0,0, true);  // Clear to black
    qp_rect(lcd, 100, 100, 120, 120, 0, 0, 100, true);
}
