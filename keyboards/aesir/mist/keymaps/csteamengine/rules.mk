USER_NAME := csteamengine
VIA_ENABLE = yes
WEAR_LEVELING_ENABLE = yes
OLED_ENABLE = no
QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += ili9341_spi
BACKLIGHT_ENABLE = yes
BOOTMAGIC_ENABLE = yes
PWM_ENABLE = yes
TAP_DANCE_ENABLE = yes
QGF_DECOMPRESSOR_ENABLE = yes

# Dynamic keymap display: hand-drawn layer art rendered live from the keymap.
SRC += keymap_display.c
SRC += ./fonts/font_proggy_tiny.qff.c   # per-key labels
SRC += ./fonts/norse20.qff.c            # top status line
SRC += ./graphics/hermod-logo.qgf.c     # logo, bottom-right
