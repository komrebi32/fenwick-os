#ifndef KBD_H
#define KBD_H

#include <stddef.h>

#define KBD_PORT        0x60
#define KBD_STATUS_PORT 0x64

#define KBD_BUFFER_SIZE 256

typedef enum {
    KBD_MOD_NONE    = 0,
    KBD_MOD_SHIFT   = (1 << 0),
    KBD_MOD_CTRL    = (1 << 1),
    KBD_MOD_ALT     = (1 << 2),
    KBD_MOD_CAPS    = (1 << 3),
    KBD_MOD_NUM     = (1 << 4),
} kbd_modifiers_t;

typedef enum {
    KBD_KEY_NONE        = 0x00,
    KBD_KEY_ESC         = 0x01,
    KBD_KEY_BACKSPACE   = 0x0E,
    KBD_KEY_TAB         = 0x0F,
    KBD_KEY_ENTER       = 0x1C,
    KBD_KEY_LCTRL       = 0x1D,
    KBD_KEY_LSHIFT      = 0x2A,
    KBD_KEY_RSHIFT      = 0x36,
    KBD_KEY_LALT        = 0x38,
    KBD_KEY_CAPSLOCK    = 0x3A,
    KBD_KEY_F1          = 0x3B,
    KBD_KEY_F2          = 0x3C,
    KBD_KEY_F3          = 0x3D,
    KBD_KEY_F4          = 0x3E,
    KBD_KEY_F5          = 0x3F,
    KBD_KEY_F6          = 0x40,
    KBD_KEY_F7          = 0x41,
    KBD_KEY_F8          = 0x42,
    KBD_KEY_F9          = 0x43,
    KBD_KEY_F10         = 0x44,
    KBD_KEY_NUMLOCK     = 0x45,
    KBD_KEY_SCROLLLOCK  = 0x46,
    KBD_KEY_HOME        = 0x47,
    KBD_KEY_UP          = 0x48,
    KBD_KEY_PGUP        = 0x49,
    KBD_KEY_LEFT        = 0x4B,
    KBD_KEY_RIGHT       = 0x4D,
    KBD_KEY_END         = 0x4F,
    KBD_KEY_DOWN        = 0x50,
    KBD_KEY_PGDN        = 0x51,
    KBD_KEY_INS         = 0x52,
    KBD_KEY_DEL         = 0x53,
    KBD_KEY_F11         = 0x57,
    KBD_KEY_F12         = 0x58,
} kbd_key_t;

void kbd_init(void);
void kbd_handler(void);
char kbd_getchar(void);
int kbd_has_input(void);
int kbd_get_modifiers(void);
int kbd_is_key_pressed(kbd_key_t key);

#endif
