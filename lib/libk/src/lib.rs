#![no_std]
#![no_main]
#![feature(alloc_error_handler)]
#![feature(global_allocator)]
#![feature(allocator_api)]

pub mod string;
pub mod stdio;
pub mod memory;
pub mod kassert;

use crate::kassert::kpanic;

#[alloc_error_handler]
fn alloc_error(_: core::alloc::Layout) -> ! {
    kpanic(b"allocation error\0".as_ptr())
}

#[panic_handler]
fn panic_handler(_: &core::panic::PanicInfo) -> ! {
    kpanic(b"panic\0".as_ptr())
}
