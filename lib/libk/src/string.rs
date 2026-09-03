use core::ptr;

#[no_mangle]
pub unsafe extern "C" fn kmemcpy(dest: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    ptr::copy_nonoverlapping(src, dest, n);
    dest
}

#[no_mangle]
pub unsafe extern "C" fn kmemset(dest: *mut u8, val: u8, n: usize) -> *mut u8 {
    ptr::write_bytes(dest, val, n);
    dest
}

#[no_mangle]
pub unsafe extern "C" fn kmemcmp(s1: *const u8, s2: *const u8, n: usize) -> i32 {
    for i in 0..n {
        let a = *s1.add(i);
        let b = *s2.add(i);
        if a != b {
            return (a as i32) - (b as i32);
        }
    }
    0
}

#[no_mangle]
pub unsafe extern "C" fn kstrlen(s: *const u8) -> usize {
    let mut len = 0usize;
    unsafe {
        while *s.add(len) != 0 {
            len += 1;
        }
    }
    len
}

#[no_mangle]
pub unsafe extern "C" fn kstrcpy(dest: *mut u8, src: *const u8) -> *mut u8 {
    let mut i = 0usize;
    loop {
        let c = *src.add(i);
        *dest.add(i) = c;
        if c == 0 {
            break;
        }
        i += 1;
    }
    dest
}

#[no_mangle]
pub unsafe extern "C" fn kstrncpy(dest: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    let mut i = 0usize;
    loop {
        if i >= n {
            return dest;
        }
        let c = *src.add(i);
        *dest.add(i) = c;
        if c == 0 {
            break;
        }
        i += 1;
    }
    dest
}

#[no_mangle]
pub unsafe extern "C" fn kstrcmp(mut s1: *const u8, mut s2: *const u8) -> i32 {
    loop {
        let a = *s1;
        let b = *s2;
        if a != b {
            return (a as i32) - (b as i32);
        }
        if a == 0 {
            return 0;
        }
        s1 = s1.add(1);
        s2 = s2.add(1);
    }
}

#[no_mangle]
pub unsafe extern "C" fn kstrncmp(s1: *const u8, s2: *const u8, n: usize) -> i32 {
    for i in 0..n {
        let a = *s1.add(i);
        let b = *s2.add(i);
        if a != b {
            return (a as i32) - (b as i32);
        }
        if a == 0 {
            return 0;
        }
    }
    0
}

#[no_mangle]
pub unsafe extern "C" fn kstrchr(s: *const u8, c: u8) -> *mut u8 {
    let mut i = 0usize;
    loop {
        let ch = *s.add(i);
        if ch == c {
            return s as *mut u8;
        }
        if ch == 0 {
            return core::ptr::null_mut();
        }
        i += 1;
    }
}

#[no_mangle]
pub unsafe extern "C" fn kstrrchr(s: *const u8, c: u8) -> *mut u8 {
    let mut last = core::ptr::null_mut();
    let mut i = 0usize;
    loop {
        let ch = *s.add(i);
        if ch == c {
            last = s as *mut u8;
        }
        if ch == 0 {
            return last;
        }
        i += 1;
    }
}

#[no_mangle]
pub unsafe extern "C" fn kstrcat(dest: *mut u8, src: *const u8) -> *mut u8 {
    let dest_len = kstrlen(dest);
    let mut i = 0usize;
    loop {
        let c = *src.add(i);
        *dest.add(dest_len + i) = c;
        if c == 0 {
            break;
        }
        i += 1;
    }
    dest
}

#[no_mangle]
pub unsafe extern "C" fn kstrncat(dest: *mut u8, src: *const u8, n: usize) -> *mut u8 {
    let dest_len = kstrlen(dest);
    let mut i = 0usize;
    loop {
        if i >= n {
            *dest.add(dest_len + n) = 0;
            return dest;
        }
        let c = *src.add(i);
        *dest.add(dest_len + i) = c;
        if c == 0 {
            break;
        }
        i += 1;
    }
    dest
}
