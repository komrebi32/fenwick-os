#![no_std]
#![no_main]

pub mod string;
pub mod stdio;
pub mod memory;
pub mod kassert;

use crate::kassert::kpanic;

#[panic_handler]
fn panic_handler(_: &core::panic::PanicInfo) -> ! {
    unsafe { kpanic(b"panic\0".as_ptr()) }
}
