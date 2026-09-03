const VGA_BUFFER: *mut u8 = 0xB8000 as *mut u8;
const VGA_WIDTH: usize = 80;
const VGA_HEIGHT: usize = 25;

static mut CURSOR_X: usize = 0;
static mut CURSOR_Y: usize = 0;
static mut CURRENT_COLOR: u8 = 0x0F;

#[inline]
fn vga_index(x: usize, y: usize) -> usize {
    (y * VGA_WIDTH + x) * 2
}

#[inline]
unsafe fn vga_putc_at(c: u8, color: u8, x: usize, y: usize) {
    let idx = vga_index(x, y);
    *VGA_BUFFER.add(idx) = c;
    *VGA_BUFFER.add(idx + 1) = color;
}

unsafe fn scroll_if_needed() {
    if CURSOR_Y >= VGA_HEIGHT {
        for y in 1..VGA_HEIGHT {
            for x in 0..VGA_WIDTH {
                let src = vga_index(x, y);
                let dst = vga_index(x, y - 1);
                *VGA_BUFFER.add(dst) = *VGA_BUFFER.add(src);
                *VGA_BUFFER.add(dst + 1) = *VGA_BUFFER.add(src + 1);
            }
        }
        for x in 0..VGA_WIDTH {
            let idx = vga_index(x, VGA_HEIGHT - 1);
            *VGA_BUFFER.add(idx) = 0;
            *VGA_BUFFER.add(idx + 1) = CURRENT_COLOR;
        }
        CURSOR_Y = VGA_HEIGHT - 1;
    }
}

#[no_mangle]
pub unsafe extern "C" fn kputc(c: u8) {
    match c {
        b'\n' => {
            CURSOR_X = 0;
            CURSOR_Y += 1;
            scroll_if_needed();
        }
        b'\r' => {
            CURSOR_X = 0;
        }
        b'\t' => {
            let tabsize = 4;
            let spaces = tabsize - (CURSOR_X % tabsize);
            for _ in 0..spaces {
                if CURSOR_X >= VGA_WIDTH {
                    CURSOR_X = 0;
                    CURSOR_Y += 1;
                    scroll_if_needed();
                }
                vga_putc_at(b' ', CURRENT_COLOR, CURSOR_X, CURSOR_Y);
                CURSOR_X += 1;
            }
        }
        b'\x08' => {
            if CURSOR_X > 0 {
                CURSOR_X -= 1;
                vga_putc_at(b' ', CURRENT_COLOR, CURSOR_X, CURSOR_Y);
            }
        }
        c if c >= 0x20 => {
            if CURSOR_X >= VGA_WIDTH {
                CURSOR_X = 0;
                CURSOR_Y += 1;
                scroll_if_needed();
            }
            vga_putc_at(c, CURRENT_COLOR, CURSOR_X, CURSOR_Y);
            CURSOR_X += 1;
        }
        _ => {}
    }
}

#[no_mangle]
pub unsafe extern "C" fn kputs(s: *const u8) {
    let mut i = 0usize;
    loop {
        let c = *s.add(i);
        if c == 0 {
            break;
        }
        kputc(c);
        i += 1;
    }
}

#[no_mangle]
pub unsafe extern "C" fn kprintf_va(fmt: *const u8, mut args: *const u64) {
    let mut i = 0usize;
    let mut in_format = false;

    loop {
        let c = *fmt.add(i);
        if c == 0 {
            break;
        }
        if in_format {
            match c {
                b's' => {
                    let s = *args as *const u8;
                    kputs(s);
                    args = args.add(1);
                }
                b'c' => {
                    kputc(*args as u8);
                    args = args.add(1);
                }
                b'd' => {
                    let val = *args as usize;
                    if val == 0 {
                        kputc(b'0');
                    } else {
                        let mut buf = [0u8; 20];
                        let mut j = buf.len();
                        let mut v = val;
                        while v > 0 {
                            j -= 1;
                            buf[j] = b'0' + (v % 10) as u8;
                            v /= 10;
                        }
                        for k in j..buf.len() {
                            kputc(buf[k]);
                        }
                    }
                    args = args.add(1);
                }
                b'x' | b'X' => {
                    let val = *args as usize;
                    kputc(b'0');
                    kputc(b'x');
                    if val == 0 {
                        kputc(b'0');
                    } else {
                        let mut buf = [0u8; 16];
                        let mut j = buf.len();
                        let mut v = val;
                        while v > 0 {
                            j -= 1;
                            let nibble = (v & 0xF) as u8;
                            buf[j] = if nibble < 10 { b'0' + nibble } else { b'a' + (nibble - 10) };
                            v >>= 4;
                        }
                        for k in j..buf.len() {
                            kputc(buf[k]);
                        }
                    }
                    args = args.add(1);
                }
                b'p' => {
                    let val = *args as usize;
                    kputc(b'0');
                    kputc(b'x');
                    let size = core::mem::size_of::<usize>();
                    let shift = (size * 8 - 4) as u32;
                    let mut j = 0usize;
                    while j < size * 2 {
                        let nibble = ((val >> (shift - (j as u32) * 4)) & 0xF) as u8;
                        kputc(if nibble < 10 { b'0' + nibble } else { b'a' + (nibble - 10) });
                        j += 1;
                    }
                    args = args.add(1);
                }
                b'%' => {
                    kputc(b'%');
                }
                _ => {
                    kputc(b'%');
                    kputc(c);
                }
            }
            in_format = false;
        } else if c == b'%' {
            in_format = true;
        } else {
            kputc(c);
        }
        i += 1;
    }
}

#[no_mangle]
pub unsafe extern "C" fn kset_color(color: u8) {
    CURRENT_COLOR = color;
}

#[no_mangle]
pub unsafe extern "C" fn kclear_screen() {
    for y in 0..VGA_HEIGHT {
        for x in 0..VGA_WIDTH {
            vga_putc_at(0, CURRENT_COLOR, x, y);
        }
    }
    CURSOR_X = 0;
    CURSOR_Y = 0;
}
