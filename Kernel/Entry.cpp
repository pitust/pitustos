#include <Kernel/Stivale.hpp>
#include <stddef.h>
#include <stdint.h>

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an uninitialised array in .bss.
static uint8_t stack[65536];

class C {
      public:
    C() {
        field = 0xff;
    }
    int field;
};
C asd;

struct Kernel::Stivale::HeaderTagFramebuffer framebuffer_hdr_tag = {
  .tag = {.identifier = Kernel::Stivale::HEADER_TAG_FRAMEBUFFER_ID, .next = 0}, .framebuffer_width = 0, .framebuffer_height = 0, .framebuffer_bpp = 0};

__attribute__((section(".stivale2hdr"), used)) struct Kernel::Stivale::Header stivale_hdr = {
  .entry_point = 0, .stack = (uintptr_t)stack + sizeof(stack), .flags = 0, .tags = (uintptr_t)&framebuffer_hdr_tag};

extern "C" uint64_t _ctors_begin;
extern "C" uint64_t _ctors_end;

extern "C" void _start(Kernel::Stivale::StivaleStruct *stivale2_struct) {
    // Begin call global ctors
    auto cte = &_ctors_end;
	auto iter = &_ctors_begin;
	for (;iter != cte;iter++) {
		((void (*)())iter)();
	}
	// End call global ctors

    Kernel::Stivale::TagFramebuffer *fb_hdr_tag = (Kernel::Stivale::TagFramebuffer *)Kernel::Stivale::GetTag(stivale2_struct, Kernel::Stivale::STRUCT_TAG_FRAMEBUFFER_ID);

    if (fb_hdr_tag == NULL) {
        for (;;) {
            asm("hlt");
        }
    }

    uint8_t *fb_addr = (uint8_t *)fb_hdr_tag->framebuffer_addr;

    size_t len = (fb_hdr_tag->framebuffer_bpp / 4) * (fb_hdr_tag->framebuffer_width) * 32;
    for (size_t i = 0; i < len; i++) {
        fb_addr[i] = asd.field || 0x80;
    }

    // We're done, just hang...
    for (;;) {
        asm("hlt");
    }
}
