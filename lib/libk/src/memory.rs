static mut HEAP_START: usize = 0;
static mut HEAP_END: usize = 0;
static mut HEAP_CURRENT: usize = 0;
static mut HEAP_INITIALIZED: bool = false;

#[no_mangle]
pub unsafe extern "C" fn kheap_init(start: usize, size: usize) {
    HEAP_START = start;
    HEAP_END = start + size;
    HEAP_CURRENT = start;
    HEAP_INITIALIZED = true;
}

#[no_mangle]
pub unsafe extern "C" fn kpage_init() {
}

#[no_mangle]
pub unsafe extern "C" fn kmalloc(size: usize) -> *mut u8 {
    if !HEAP_INITIALIZED || HEAP_START == 0 {
        return core::ptr::null_mut();
    }
    let align = 8;
    let current = HEAP_CURRENT;
    let offset = (align - (current % align)) % align;
    let aligned = current + offset;
    let end = aligned.checked_add(size);
    match end {
        Some(e) if e <= HEAP_END => {
            HEAP_CURRENT = e;
            aligned as *mut u8
        }
        _ => core::ptr::null_mut(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kmalloc_aligned(size: usize, align: usize) -> *mut u8 {
    if !HEAP_INITIALIZED || HEAP_START == 0 {
        return core::ptr::null_mut();
    }
    if align < 1 || !align.is_power_of_two() {
        return core::ptr::null_mut();
    }
    let current = HEAP_CURRENT;
    let offset = (align - (current % align)) % align;
    let aligned = current + offset;
    let end = aligned.checked_add(size);
    match end {
        Some(e) if e <= HEAP_END => {
            HEAP_CURRENT = e;
            aligned as *mut u8
        }
        _ => core::ptr::null_mut(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn kfree(_ptr: *mut u8) {
}

#[no_mangle]
pub unsafe extern "C" fn krealloc(ptr: *mut u8, old_size: usize, new_size: usize) -> *mut u8 {
    if ptr.is_null() {
        return kmalloc(new_size);
    }
    if new_size == 0 {
        kfree(ptr);
        return core::ptr::null_mut();
    }
    let new_ptr = kmalloc(new_size);
    if !new_ptr.is_null() {
        let copy_size = if old_size < new_size { old_size } else { new_size };
        core::ptr::copy_nonoverlapping(ptr, new_ptr, copy_size);
        kfree(ptr);
    }
    new_ptr
}
