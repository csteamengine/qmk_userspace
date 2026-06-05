USER_NAME := csteamengine
VIA_ENABLE = yes
WEAR_LEVELING_ENABLE = yes
OLED_ENABLE = no
QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += ili9341_spi
BACKLIGHT_ENABLE = yes
BOOTMAGIC_ENABLE = yes
PWM_ENABLE = yes
SRC += ./fonts/norse20.qff.c
SRC += ./graphics/hermod-logo.qgf.c
SRC += ./graphics/left-base-layout.qgf.c
SRC += ./graphics/left-1-layout.qgf.c
SRC += ./graphics/mist/layer-1.qgf.c
SRC += ./graphics/mist/layer-1-full.qgf.c
SRC += ./graphics/mist/layer-2-full.qgf.c
TAP_DANCE_ENABLE = yes
QGF_DECOMPRESSOR_ENABLE = yes

# Dynamic keymap display: mirrors the physical layout from g_led_config.
SRC += keymap_display.c
SRC += ./fonts/font_proggy_tiny.qff.c
