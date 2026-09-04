use crate::stdio::{kclear_screen, kputs, kset_color};

#[no_mangle]
pub unsafe extern "C" fn kpanic(msg: *const u8) -> ! {
    static mut PANICED: bool = false;

    if PANICED {
        loop {}
    }
    PANICED = true;

    kset_color(0x1F);
    kclear_screen();
    kputs(b"\n*** KERNEL PANIC ***\n\0".as_ptr());
    kputs(msg);
    kputs(b"\nSystem halted.\0".as_ptr());

    loop {}
}

#[no_mangle]
#[cold]
pub unsafe extern "C" fn kassert(cond: u32, msg: *const u8) {
    if cond == 0 {
        kputs(b"ASSERTION FAILED: \0".as_ptr());
        kputs(msg);
        kputs(b"\n\0".as_ptr());
        kpanic(b"assertion failed\0".as_ptr());
    }
}
