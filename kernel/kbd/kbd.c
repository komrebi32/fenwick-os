#include "kbd.h"
#include <libk.h>

static const char kbd_scancode_lower[128] = {
    [0x00] = 0,
    [0x01] = 27, [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9',
    [0x0B] = '0', [0x0C] = '-', [0x0D] = '=', [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x37] = '*',
    [0x39] = ' ',
    [0x56] = '|',
};

static const char kbd_scancode_upper[128] = {
    [0x00] = 0,
    [0x01] = 27, [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(',
    [0x0B] = ')', [0x0C] = '_', [0x0D] = '+', [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R', [0x14] = 'T',
    [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}', [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
    [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L', [0x27] = ':',
    [0x28] = '"', [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V', [0x30] = 'B',
    [0x31] = 'N', [0x32] = 'M', [0x33] = '<', [0x34] = '>', [0x35] = '?',
    [0x37] = '*',
    [0x39] = ' ',
};

static char kbd_buffer[KBD_BUFFER_SIZE];
static int kbd_buffer_start = 0;
static int kbd_buffer_end = 0;
static int kbd_modifiers = 0;
static int kbd_pressed[128] = {0};
static int kbd_extended = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void kbd_init(void) {
    for (int i = 0; i < KBD_BUFFER_SIZE; i++) {
        kbd_buffer[i] = 0;
    }
    kbd_buffer_start = 0;
    kbd_buffer_end = 0;
    kbd_modifiers = 0;
    kbd_extended = 0;

    kbd_pressed[0x3A] = 0;
}

static void kbd_buffer_push(char c) {
    if (c == 0) return;
    int next = (kbd_buffer_end + 1) % KBD_BUFFER_SIZE;
    if (next == kbd_buffer_start) return;
    kbd_buffer[kbd_buffer_end] = c;
    kbd_buffer_end = next;
}

void kbd_handler(void) {
    uint8_t scancode = inb(KBD_PORT);

    if (scancode == 0xE0) {
        kbd_extended = 1;
        return;
    }

    int released = scancode & 0x80;
    uint8_t code = scancode & 0x7F;
    kbd_extended = 0;

    if (code >= 128) return;

    if (released) {
        kbd_pressed[code] = 0;
        if (code == 0x2A || code == 0x36) {
            kbd_modifiers &= ~KBD_MOD_SHIFT;
        } else if (code == 0x1D) {
            kbd_modifiers &= ~KBD_MOD_CTRL;
        } else if (code == 0x38) {
            kbd_modifiers &= ~KBD_MOD_ALT;
        }
        return;
    }

    kbd_pressed[code] = 1;

    if (code == 0x2A || code == 0x36) {
        kbd_modifiers |= KBD_MOD_SHIFT;
        return;
    }
    if (code == 0x1D) {
        kbd_modifiers |= KBD_MOD_CTRL;
        return;
    }
    if (code == 0x38) {
        kbd_modifiers |= KBD_MOD_ALT;
        return;
    }
    if (code == 0x3A) {
        kbd_modifiers ^= KBD_MOD_CAPS;
        return;
    }

    int shift = (kbd_modifiers & KBD_MOD_SHIFT) ? 1 : 0;
    int caps = (kbd_modifiers & KBD_MOD_CAPS) ? 1 : 0;

    char c = 0;
    if (shift) {
        c = kbd_scancode_upper[code];
    } else {
        c = kbd_scancode_lower[code];
    }

    if (c >= 'a' && c <= 'z') {
        if (caps) c = c - 'a' + 'A';
    } else if (c >= 'A' && c <= 'Z') {
        if (!shift && caps) c = c - 'A' + 'a';
    }

    kbd_buffer_push(c);
}

char kbd_getchar(void) {
    if (kbd_buffer_start == kbd_buffer_end) return 0;
    char c = kbd_buffer[kbd_buffer_start];
    kbd_buffer_start = (kbd_buffer_start + 1) % KBD_BUFFER_SIZE;
    return c;
}

int kbd_has_input(void) {
    return kbd_buffer_start != kbd_buffer_end;
}

int kbd_get_modifiers(void) {
    return kbd_modifiers;
}

int kbd_is_key_pressed(kbd_key_t key) {
    if (key >= 128) return 0;
    return kbd_pressed[key];
}
