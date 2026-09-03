use core::alloc::{GlobalAlloc, Layout};
use core::ptr;

static mut HEAP_START: usize = 0;
static mut HEAP_END: usize = 0;
static mut HEAP_CURRENT: usize = 0;
static mut HEAP_INITIALIZED: bool = false;

struct BumpAllocator;

unsafe impl GlobalAlloc for BumpAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        if !HEAP_INITIALIZED || HEAP_START == 0 {
            return core::ptr::null_mut();
        }
        let align = layout.align();
        let size = layout.size();
        let mut current = HEAP_CURRENT;
        let offset = (align - (current % align)) % align;
        let new_current = current.checked_add(offset).and_then(|v| v.checked_add(size));
        match new_current {
            Some(end) if end <= HEAP_END => {
                let ptr = (current + offset) as *mut u8;
                HEAP_CURRENT = end;
                ptr
            }
            _ => core::ptr::null_mut(),
        }
    }

    unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {}
}

#[global_allocator]
static GLOBAL: BumpAllocator = BumpAllocator;

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
    let layout = Layout::from_size_align(size, 8).unwrap_or(Layout::from_size_align(1, 1).unwrap());
    GLOBAL.alloc(layout)
}

#[no_mangle]
pub unsafe extern "C" fn kmalloc_aligned(size: usize, align: usize) -> *mut u8 {
    let layout = Layout::from_size_align(size, align).unwrap_or(Layout::from_size_align(1, 1).unwrap());
    GLOBAL.alloc(layout)
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
        ptr::copy_nonoverlapping(ptr, new_ptr, copy_size);
        kfree(ptr);
    }
    new_ptr
}
