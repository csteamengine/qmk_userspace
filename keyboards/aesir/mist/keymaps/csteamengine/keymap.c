// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "csteamengine.h"
#include "lib/layer_status/layer_status.h"
#include "print.h"

// Dynamic, layer-aware keymap render on the LCD.
#include "keymap_display.h"

#ifdef LCD_ACTIVITY_TIMEOUT
#include "backlight.h"
#include <qp.h>

painter_device_t lcd;
#endif

#ifdef BACKLIGHT_ENABLE
static uint8_t last_backlight = 255;
#endif

#ifdef SECRETS_H
#include "secrets.h"
#endif

enum custom_keycodes {
    CS_MPLY = SAFE_RANGE,
    CS_MNXT,
    CS_MPRV,
    CS_UP,
    CS_DOWN,
};

#ifdef TAP_DANCE_ENABLE

enum {
    TASK_EMOJI_LAYER, // Our custom tap dance key; add any other tap dance keys to this enum
    TD_PW_ONE,
    TD_PW_TWO,
    TD_PW_THREE,
    TD_PW_FOUR,
    TD_CAPS_LOCK,
    TD_SC_LSHIFT,
    TD_SC_RSHIFT,
    TD_TICK_TICK,
    TD_OBSIDIAN,
    TD_COPY_FILE,
    TD_VOL_UP,
    TD_VOL_DOWN,
    TD_ENTER_SUPER,
    TD_BRACKET_GUI,
    TD_LPAR_GUI,
    TD_RPAR_GUI,
    TD_LALT_PAR,
    TD_LCTL_PAR,
    TD_RALT_PAR,
    TD_RCTL_PAR,
    TD_ESC_CAPS,
    TD_GRV_LAYER,
    TD_REFACTOR,
    TD_FN0_CMD_SHIFT_SPACE
};

typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP,
    TD_TRIPLE_TAP
} td_state_t;

typedef struct {
    bool is_press_action;
    td_state_t state;
    uint16_t single_tap_keycode;
    uint16_t double_tap_keycode;
    uint16_t held;
    uint16_t hold;
} td_tap_t;
#endif

// Abbreviated (<=3 char) labels for keymap_display.c. Defined here so it can see
// the tap-dance enum and the custom keycodes.
const char *keycode_label(uint16_t keycode) {
    switch (keycode) {
        // shared / base
        case KC_ESC:  return "ESC";
        case KC_DEL:  return "DEL";
        case KC_MPRV: return "PRV";
        case KC_MPLY: return "PLY";
        case KC_MNXT: return "NXT";
        case KC_SPC:  return "SPC";
        case KC_RCTL: return "CTL";
        case KC_LOPT: return "OPT";
        case KC_LGUI: return "CMD";
        case KC_UP:   return "UP";
        case KC_DOWN: return "DN";
        case KC_LEFT: return "LT";
        case KC_RIGHT:return "RT";
        case KC_HOME: return "HOM";
        case KC_END:  return "END";
        case KC_VOLU: return "VL+";
        case KC_VOLD: return "VL-";
        // custom keycodes
        case SCREEN_RECORDING_1:       return "REC";
        case SCREEN_SHOT_TO_CLIPBOARD: return "CLP";
        case SCREEN_SHOT_AREA:         return "ARA";
        case SCREEN_SHOT_SCREEN:       return "SHT";
        case MAC_EMOJIS:               return "EMO";
        case APP_SWITCHER:             return "APP";
        // tap dances
        case TD(TD_PW_ONE):              return "PW1";
        case TD(TD_PW_TWO):              return "PW2";
        case TD(TD_PW_THREE):            return "PW3";
        case TD(TD_PW_FOUR):             return "PW4";
        case TD(TD_TICK_TICK):           return "TOD";
        case TD(TD_OBSIDIAN):            return "NOT";
        case TD(TD_REFACTOR):            return "RFC";
        case TD(TD_FN0_CMD_SHIFT_SPACE): return "LYR";
        // FN0 layer
        case QK_BOOT: return "BL";
        case RM_TOGG: return "TOG";
        case RM_VALD: return "V-";
        case RM_VALU: return "V+";
        case RM_HUEU: return "HUE";
        case RM_PREV: return "RP-";
        case RM_NEXT: return "RP+";
        case LAG(KC_LEFT):  return "CB";
        case LAG(KC_RIGHT): return "CF";
        // empty cells
        case KC_NO:
        case KC_TRNS:
        default: return "";
    }
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_ESC,     SCREEN_RECORDING_1,   SCREEN_SHOT_TO_CLIPBOARD, SCREEN_SHOT_AREA,   SCREEN_SHOT_SCREEN,    KC_NO, KC_MPRV,   KC_MPLY,   KC_MNXT,   TD(TD_PW_FOUR),   TD(TD_PW_THREE), TD(TD_PW_TWO),  TD(TD_PW_ONE), KC_DEL,
        KC_GRV,   KC_1,    KC_2,    KC_3,    KC_4,    KC_5,     KC_6,    KC_7,    KC_8,      KC_9,    KC_0,    KC_MINS, KC_EQL,                 KC_BSPC,
        KC_TAB,   KC_Q,     KC_W,   KC_E,    KC_R,    KC_T,     KC_Y,    KC_U,    KC_I,      KC_O,    KC_P,    KC_LBRC, KC_RBRC,                KC_BSLS,                        KC_NUM,     KC_PSLS, KC_PAST, KC_PMNS,
        KC_CAPS,    KC_A,     KC_S,   KC_D,    KC_F,    KC_G,     KC_H,    KC_J,    KC_K,      KC_L,    KC_SCLN, KC_QUOT,                       KC_ENT,                         KC_P7,      KC_P8,   KC_P9,
            KC_LSFT,        KC_Z,     KC_X,   KC_C,    KC_V,    KC_B,     KC_N,    KC_M,    KC_COMM,   KC_DOT,  KC_SLSH,                        KC_RSFT,                        KC_P4,      KC_P5,   KC_P6,   KC_PPLS,
        KC_NO,  KC_NO,    MAC_EMOJIS, KC_LOPT,  KC_LGUI,      TD(TD_FN0_CMD_SHIFT_SPACE),      KC_SPC, KC_RCTL,           KC_UP,                       TD(TD_REFACTOR),   KC_NO,                  KC_P1,      KC_P2,   KC_P3,
        TD(TD_TICK_TICK),   TD(TD_OBSIDIAN),      APP_SWITCHER,                             KC_LEFT,    KC_DOWN,    KC_RIGHT,        KC_HOME,           KC_END,                 KC_P0,               KC_PDOT, KC_PENT
    ),
    [_FN0] = LAYOUT(
        QK_BOOT,     RM_TOGG, RM_VALD,    RM_VALU,    RM_HUEU,    RM_PREV,        RM_NEXT,   _______,   _______,   _______,   _______,  _______,  _______,  _______,
        _______,     _______,    _______,    _______,    _______,    _______,     _______,    _______,    _______,      _______,    _______,    _______,   _______,     _______,
        _______,     _______,     _______,   _______,    _______,    _______,     _______,    _______,    _______,      _______,    _______,    _______,   _______,     _______,    _______,    _______,    _______,    _______,
        _______,        _______,     _______,   _______,    _______,    _______,     _______,    _______,    _______,      _______,    _______,    _______,                         _______,    _______,    _______,    _______,
                _______,        _______,     _______,   _______,    _______,    _______,     _______,    _______,    _______,   _______,  _______,     _______,                     _______,    _______,    _______,    _______,
        _______,    _______,    _______,    _______,  _______,  _______,    KC_HOME,     KC_END,            KC_VOLU,                      _______,      _______,                    _______,    _______,    _______,
        _______,    _______,    _______,                                                   LAG(KC_LEFT),    KC_VOLD,    LAG(KC_RIGHT),    _______,      _______,                    _______,                _______,    _______
    ),
};

// clang-format on

#ifdef LCD_ACTIVITY_TIMEOUT
static void check_lcd_timeout(void); // checks if enough time has passed for the LCD to timeout
bool        is_lcd_timeout = false;  // store if the LCD has timed out or not
bool        peripherals_on = true;
static void init_lcd(void);
static void power_off_lcd(void);
#endif

static bool is_held = false; // used by the refactor tap dance (below)

#ifdef LCD_ACTIVITY_TIMEOUT
// current_rgb_mode / current_layer_name are reused by keymap_display.c's status line.

const char *current_rgb_mode(void) {
    switch (rgb_matrix_get_mode()) {
        case RGB_MATRIX_BREATHING:
            return "Breathing";
        case RGB_MATRIX_CYCLE_ALL:
            return "Cycle All";
        case RGB_MATRIX_CYCLE_LEFT_RIGHT:
            return "Cycle L-R";
        case RGB_MATRIX_HUE_BREATHING:
            return "Hue Breath";
        case RGB_MATRIX_JELLYBEAN_RAINDROPS:
            return "Jellybean";
        case RGB_MATRIX_MULTISPLASH:
            return "MultiSplash";
        case RGB_MATRIX_PIXEL_FLOW:
            return "Pixel Flow";
        case RGB_MATRIX_PIXEL_RAIN:
            return "Pixel Rain";
        case RGB_MATRIX_RAINBOW_BEACON:
            return "RB Beacon";
        case RGB_MATRIX_RAINBOW_MOVING_CHEVRON:
            return "RB Chevron";
        case RGB_MATRIX_RAINBOW_PINWHEELS:
            return "RB Pinwheel";
        case RGB_MATRIX_RAINDROPS:
            return "Raindrops";
        case RGB_MATRIX_SOLID_COLOR:
            return "Solid Color";
        case RGB_MATRIX_SOLID_MULTISPLASH:
            return "Solid MSplash";
        case RGB_MATRIX_SOLID_REACTIVE:
            return "Solid React";
        case RGB_MATRIX_SOLID_REACTIVE_CROSS:
            return "Solid R Cross";
        case RGB_MATRIX_SOLID_REACTIVE_SIMPLE:
            return "Solid R Simple";
        case RGB_MATRIX_SOLID_REACTIVE_WIDE:
            return "Solid R Wide";
        case RGB_MATRIX_SOLID_SPLASH:
            return "Solid Splash";
        case RGB_MATRIX_SPLASH:
            return "Splash";
        case RGB_MATRIX_TYPING_HEATMAP:
            return "Heatmap";
        default:
            return "Unknown";
    }
}


const char *current_layer_name(void) {
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            return "Base";
        case _FN0:
            return "Layer 1";
    }
    return "unknown";
}

void init_lcd(void) {
    qp_init(lcd, QP_ROTATION_90);

    // Turn LCD On
    qp_power(lcd, true);

    backlight_set(10);

    // Turn on the LCD and clear the display
    qp_rect(lcd, 0, 0, 320, 240, 6, 0, 0, true);
}

void power_off_lcd(void) {
    // Turn on the LCD and clear the display
    // qp_power(lcd, false);
    qp_rect(lcd, 0, 0, 320, 240, 0, 0, 0, true); // fill with black
    qp_flush(lcd);
    qp_power(lcd, false);

    backlight_set(0);

    // TODO Disable LCD backlight
}

void check_lcd_timeout(void) {
    peripherals_on = last_input_activity_elapsed() < LCD_ACTIVITY_TIMEOUT;

    if (!is_lcd_timeout && !peripherals_on) // check if RGB has already timeout and if enough time has passed
    {
        #ifdef BACKLIGHT_ENABLE
        if (last_backlight == 255) {
            last_backlight = get_backlight_level();
        }

        backlight_set(0);
        #endif

        power_off_lcd();

        is_lcd_timeout = true;
    } else if(is_lcd_timeout && peripherals_on) {
        init_lcd();
        keymap_display_init(lcd);
        is_lcd_timeout = false;
    }
}
#endif

void keyboard_pre_init_keymap(void) {
    #ifdef LCD_ACTIVITY_TIMEOUT
        // TODO update LCD backlight PWM
        // Turn LCD Off for now
        backlight_set(0);
    #endif
}

void keyboard_post_init_keymap(void) {
    #ifdef LCD_ACTIVITY_TIMEOUT

    // Initialize the LCD
    lcd = qp_ili9341_make_spi_device(320, 240, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 8, 0);

    init_lcd();
    keymap_display_init(lcd);

    #ifdef BACKLIGHT_ENABLE
    backlight_enable();
    backlight_set(10);
    #endif
    #endif
}

void housekeeping_task_keymap(void) {
    #ifdef LCD_ACTIVITY_TIMEOUT
        check_lcd_timeout();

        if(!is_lcd_timeout && peripherals_on) {
            keymap_display_render(lcd);
        }
    #endif
}

void suspend_power_down_keymap(void) {
    #ifdef BACKLIGHT_ENABLE
    if (last_backlight == 255) {
        last_backlight = get_backlight_level();
    }
    backlight_set(0);
    #endif

    #ifdef LCD_ACTIVITY_TIMEOUT
        // Turn LCD Off
        power_off_lcd();
    #endif
}

void suspend_wakeup_init_keymap(void) {
    #ifdef LCD_ACTIVITY_TIMEOUT
        // Turn LCD On
        init_lcd();
        keymap_display_init(lcd);
    #endif

    #ifdef BACKLIGHT_ENABLE
    if (last_backlight != 255) {
        backlight_set(last_backlight);
    }
    last_backlight = 100;
    #endif
}

bool process_record_keymap (uint16_t keycode, keyrecord_t *record) {

      return true;
}

/// ================ TAP DANCE CONFIG =================
#ifdef TAP_DANCE_ENABLE
td_state_t cur_dance(tap_dance_state_t *state);

// Functions associated with individual tap dances
void ql_finished(tap_dance_state_t *state, void *user_data, uint16_t key_code);
void ql_reset(tap_dance_state_t *state, void *user_data);

// Determine the current tap dance state
td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (!state->pressed)
            return TD_SINGLE_TAP;
        else
            return TD_SINGLE_HOLD;
    } else if (state->count == 2)
        return TD_DOUBLE_TAP;
    else if (state->count == 3)
        return TD_TRIPLE_TAP;
    else
        return TD_UNKNOWN;
}

// Initialize tap structure associated with example tap dance key
static td_tap_t ql_tap_state = {.is_press_action = true, .state = TD_NONE};

// Functions that control what our tap dance key does
void task_layer_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LOPT(SS_LSFT(SS_TAP(X_A))));
            break;
        case TD_SINGLE_HOLD:
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            // Check to see if the layer is already set
            if (layer_state_is(_FN0)) {
                // If already set, then switch it off
                layer_off(_FN0);
            } else {
                // If not already set, then switch the layer on
                layer_on(_FN0);
            }
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
// void task_caps_lock(tap_dance_state_t *state, void *user_data) {
//     ql_tap_state.state = cur_dance(state);
//     switch (ql_tap_state.state) {
//         case TD_SINGLE_TAP:
//             tap_code(CAPS_WORD);
//             break;
//         case TD_SINGLE_HOLD:
//             break;
//         case TD_DOUBLE_TAP:
//             tap_code(KC_CAPS);
//             break;
//         default:
//             break;
//     }
// }

// Functions that control what our tap dance key does
void task_sc_left_shift(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING("(");
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_LSFT);
            break;
        case TD_DOUBLE_TAP:
            tap_code(KC_LSFT);
            tap_code(KC_LSFT);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void task_sc_right_shift(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(")");
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_RSFT);
            break;
        case TD_DOUBLE_TAP:
            tap_code(KC_RSFT);
            tap_code(KC_RSFT);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void pw_one_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code(KC_ESC);
            break;
        case TD_SINGLE_HOLD:
            SEND_STRING(SS_LGUI(SS_LCTL(SS_TAP(X_Q))));
            break;
        case TD_DOUBLE_TAP:
            #ifdef SECRETS_H
            SEND_STRING(PW_ONE_STRING);
            #endif
            break;
        case TD_TRIPLE_TAP:
            #ifdef SECRETS_H
            SEND_STRING(PW_FIVE_STRING);
            #endif
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void pw_two_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:

            break;
        case TD_SINGLE_HOLD:
            // TODO define a hold action for pw two
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            #ifdef SECRETS_H
            SEND_STRING(PW_TWO_STRING);
            #endif
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void pw_three_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:

            break;
        case TD_SINGLE_HOLD:
            // TODO define a hold action for pw three
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            #ifdef SECRETS_H
            SEND_STRING(PW_THREE_STRING);
            #endif
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void pw_four_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:

            break;
        case TD_SINGLE_HOLD:
            // TODO define a hold action for pw four
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            #ifdef SECRETS_H
            SEND_STRING(PW_FOUR_STRING);
            #endif
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void tick_tick_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LGUI(SS_LOPT(SS_LSFT(SS_TAP(X_A)))));
            break;
        case TD_SINGLE_HOLD:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_O))));
            break;
        case TD_DOUBLE_TAP:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_E))));
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void obsidian_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_LOPT(SS_LCTL(SS_TAP(X_O))))));
            break;
        case TD_SINGLE_HOLD:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_M))));
            break;
        case TD_DOUBLE_TAP:
            SEND_STRING(SS_LGUI(SS_LOPT(SS_LSFT(SS_TAP(X_O)))));
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void copy_file_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LGUI(SS_LOPT(SS_LCTL(SS_LSFT(SS_TAP(X_C))))));
            break;
        case TD_SINGLE_HOLD:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_P))));
            break;
        case TD_DOUBLE_TAP:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_LCTL(SS_TAP(X_C)))));
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void enter_super_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code(KC_PENT);
            break;
        case TD_SINGLE_HOLD:
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void l_par_gui_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_LPRN);
            break;
        case TD_SINGLE_HOLD:
            layer_on(_FN0);
            register_code(KC_LGUI);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_LBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void r_par_gui_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_RPRN);
            break;
        case TD_SINGLE_HOLD:
            layer_on(_FN0);
            register_code(KC_RGUI);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_RBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void l_alt_par_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_LPRN);
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_LALT);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_LBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void l_ctl_par_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_RPRN);
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_LCTL);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_RBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void r_alt_par_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_LPRN);
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_RALT);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_LBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void r_ctl_par_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_RPRN);
            break;
        case TD_SINGLE_HOLD:
            register_code(KC_RCTL);
            break;
        case TD_DOUBLE_TAP:
            tap_code16(KC_RBRC);
            break;
        default:
            break;
    }
}

// Functions that control what our tap dance key does
void refactor_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LSFT(SS_TAP(X_F6)));
            break;
        case TD_SINGLE_HOLD:
            is_held = true;
            break;
        case TD_DOUBLE_TAP:
        default:
            break;
    }
}

// Tap dance for FN0 layer with Cmd+Shift+Space on tap
void fn0_cmd_shift_space_finished(tap_dance_state_t *state, void *user_data) {
    ql_tap_state.state = cur_dance(state);
    switch (ql_tap_state.state) {
        case TD_SINGLE_TAP:
            SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_SPC))));
            break;
        case TD_SINGLE_HOLD:
            layer_on(_FN0);
            break;
        case TD_DOUBLE_TAP:
            break;
        default:
            break;
    }
}

void ql_reset(tap_dance_state_t *state, void *user_data) {
    // If the key was held down and now is released then switch off the layer
    if (ql_tap_state.state == TD_SINGLE_HOLD) {
        layer_on(_BASE);
        layer_off(_FN0);
        unregister_code(KC_LSFT);
        unregister_code(KC_RSFT);
        unregister_code(KC_LGUI);
        unregister_code(KC_RGUI);
        unregister_code(KC_LCTL);
        unregister_code(KC_RCTL);
        unregister_code(KC_LALT);
        unregister_code(KC_RALT);
        is_held = false;
    }
    ql_tap_state.state = TD_NONE;
}

// clang-format off
// Associate our tap dance key with its functionality
tap_dance_action_t tap_dance_actions[] = {
    [TASK_EMOJI_LAYER] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, task_layer_finished, ql_reset),
    // [TD_CAPS_LOCK] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, task_caps_lock, ql_reset),
    [TD_SC_LSHIFT] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, task_sc_left_shift, ql_reset),
    [TD_SC_RSHIFT] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, task_sc_right_shift, ql_reset),
    [TD_PW_ONE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, pw_one_finished, ql_reset),
    [TD_PW_TWO] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, pw_two_finished, ql_reset),
    [TD_PW_THREE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, pw_three_finished, ql_reset),
    [TD_PW_FOUR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, pw_four_finished, ql_reset),
    [TD_TICK_TICK] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, tick_tick_finished, ql_reset),
    [TD_OBSIDIAN] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, obsidian_finished, ql_reset),
    [TD_COPY_FILE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, copy_file_finished, ql_reset),
    [TD_VOL_UP] = ACTION_TAP_DANCE_DOUBLE(KC_KB_VOLUME_UP, KC_MPLY),
    [TD_VOL_DOWN] = ACTION_TAP_DANCE_DOUBLE(KC_KB_VOLUME_DOWN, KC_KB_MUTE),
    [TD_ENTER_SUPER] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, enter_super_finished, ql_reset),
    [TD_ESC_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_ESC, KC_CAPS),
    [TD_GRV_LAYER] = ACTION_TAP_DANCE_LAYER_MOVE(KC_GRV, _FN0),
    [TD_LPAR_GUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, l_par_gui_finished, ql_reset),
    [TD_RPAR_GUI] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, r_par_gui_finished, ql_reset),
    [TD_LALT_PAR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, l_alt_par_finished, ql_reset),
    [TD_LCTL_PAR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, l_ctl_par_finished, ql_reset),
    [TD_RALT_PAR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, r_alt_par_finished, ql_reset),
    [TD_RCTL_PAR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, r_ctl_par_finished, ql_reset),
    [TD_REFACTOR] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, refactor_finished, ql_reset),
    [TD_FN0_CMD_SHIFT_SPACE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, fn0_cmd_shift_space_finished, ql_reset),
    // TODO add more tap dance actions here.
};
// clang-format on

// Set a long-ish tapping term for tap-dance keys
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case QK_TAP_DANCE ... QK_TAP_DANCE_MAX:
            return 275;
        default:
            return TAPPING_TERM;
    }
}
#endif
/// ================ END TAP DANCE CONFIG =================
