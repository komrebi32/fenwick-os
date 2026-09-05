#include "drv_kbd.h"
#include <libk.h>

#define KBD_PORT_DATA    0x60
#define KBD_PORT_STATUS  0x64
#define BUFFER_SIZE      256

static char kbd_buffer[BUFFER_SIZE];
static int  buf_start = 0;
static int  buf_end   = 0;
static int  shift     = 0;
static int  ctrl      = 0;
static int  alt       = 0;
static int  caps      = 0;
static int  ext       = 0;
static uint8_t kbd_state[128];

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static int kbd_has_data(void) {
    return (inb(KBD_PORT_STATUS) & 0x01) != 0;
}

static void buf_push(char c) {
    if (c == 0) return;
    int next = (buf_end + 1) % BUFFER_SIZE;
    if (next == buf_start) return;
    kbd_buffer[buf_end] = c;
    buf_end = next;
}

static char scancode_to_ascii(uint8_t sc, int shift, int caps) {
    static const char lower[128] = {
        [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',
        [0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',
        [0x0C]='-',[0x0D]='=',[0x0E]='\b',[0x0F]='\t',
        [0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',
        [0x15]='y',[0x16]='u',[0x17]='i',[0x18]='o',[0x19]='p',
        [0x1A]='[',[0x1B]=']',[0x1C]='\n',
        [0x1E]='a',[0x1F]='s',[0x20]='d',[0x21]='f',[0x22]='g',
        [0x23]='h',[0x24]='j',[0x25]='k',[0x26]='l',[0x27]=';',
        [0x28]='\'',[0x29]='`',
        [0x2B]='\\',
        [0x2C]='z',[0x2D]='x',[0x2E]='c',[0x2F]='v',[0x30]='b',
        [0x31]='n',[0x32]='m',[0x33]=',',[0x34]='.',[0x35]='/',
        [0x39]=' ',
    };
    static const char upper[128] = {
        [0x02]='!',[0x03]='@',[0x04]='#',[0x05]='$',[0x06]='%',
        [0x07]='^',[0x08]='&',[0x09]='*',[0x0A]='(',[0x0B]=')',
        [0x0C]='_',[0x0D]='+',[0x0E]='\b',[0x0F]='\t',
        [0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',
        [0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',
        [0x1A]='{',[0x1B]='}',[0x1C]='\n',
        [0x1E]='A',[0x1F]='S',[0x20]='D',[0x21]='F',[0x22]='G',
        [0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=':',
        [0x28]='"',[0x29]='~',
        [0x2B]='|',
        [0x2C]='Z',[0x2D]='X',[0x2E]='C',[0x2F]='V',[0x30]='B',
        [0x31]='N',[0x32]='M',[0x33]='<',[0x34]='>',[0x35]='?',
        [0x39]=' ',
    };

    if (sc >= 128) return 0;
    char c = shift ? upper[sc] : lower[sc];
    if (caps) {
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        else if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    }
    return c;
}

void kbd_handler_isr(void) {
    if (!kbd_has_data()) return;

    uint8_t sc = inb(KBD_PORT_DATA);
    if (sc == 0xE0) { ext = 1; return; }

    int released = sc & 0x80;
    uint8_t code = sc & 0x7F;
    ext = 0;

    if (code >= 128) return;
    kbd_state[code] = released ? 0 : 1;

    if (code == 0x2A || code == 0x36) {
        shift = released ? 0 : 1;
        return;
    }
    if (code == 0x1D) {
        ctrl = released ? 0 : 1;
        return;
    }
    if (code == 0x38) {
        alt = released ? 0 : 1;
        return;
    }
    if (code == 0x3A && !released) {
        caps = !caps;
        return;
    }

    if (released) return;

    char c = scancode_to_ascii(code, shift, caps);
    buf_push(c);
}

int kbd_driver_init(device_t *dev) {
    (void)dev;
    buf_start = 0;
    buf_end = 0;
    shift = ctrl = alt = caps = ext = 0;
    for (int i = 0; i < 128; i++) kbd_state[i] = 0;
    return 0;
}

int kbd_driver_probe(device_t *dev) {
    (void)dev;
    return 0;
}

int kbd_driver_read(device_t *dev, void *buf, uint64_t len) {
    (void)dev;
    if (!buf || len == 0) return -1;

    uint64_t count = 0;
    char *dst = (char *)buf;

    while (count < len && buf_start != buf_end) {
        dst[count++] = kbd_buffer[buf_start];
        buf_start = (buf_start + 1) % BUFFER_SIZE;
    }
    return (int)count;
}

int kbd_driver_ioctl(device_t *dev, uint32_t cmd, uint64_t arg) {
    (void)dev;
    switch (cmd) {
        case 0: return shift;
        case 1: return ctrl;
        case 2: return alt;
        case 3: return caps;
        case 5: return buf_start != buf_end ? 1 : 0;
        case 10: return kbd_state[(uint8_t)arg];
        default: return -1;
    }
}

drv_ops_t kbd_ops = {
    .name     = "ps2-keyboard",
    .type     = DRV_TYPE_KEYBOARD,
    .init     = kbd_driver_init,
    .probe    = kbd_driver_probe,
    .read     = kbd_driver_read,
    .write    = 0,
    .ioctl    = kbd_driver_ioctl,
    .shutdown = 0,
};

driver_t kbd_driver = {
    .name = "ps2-keyboard",
    .type = DRV_TYPE_KEYBOARD,
    .ops  = &kbd_ops,
};
