#include <Kernel/Stivale.hpp>
#include <stddef.h>
#include <stdint.h>

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an uninitialised array in .bss.
static uint8_t stack[65536];

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

// As an example header tag, we're gonna define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode.
struct Kernel::Stivale::HeaderTagFramebuffer framebuffer_hdr_tag = {
  // All tags need to begin with an identifier and a pointer to the next tag.
  .tag =
    {// Identification constant defined in stivale2.h and the specification.
     .identifier = Kernel::Stivale::HEADER_TAG_FRAMEBUFFER_ID,
     // If next is 0, then this marks the end of the linked list of tags.
     .next = 0},
  // We set all the framebuffer specifics to 0 as we want the bootloader
  // to pick the best it can.
  .framebuffer_width  = 0,
  .framebuffer_height = 0,
  .framebuffer_bpp    = 0};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used)) struct Kernel::Stivale::Header stivale_hdr = {
  // The entry_point member is used to specify an alternative entry
  // point that the bootloader should jump to instead of the executable's
  // ELF entry point. We do not care about that so we leave it zeroed.
  .entry_point = 0,
  // Let's tell the bootloader where our stack is.
  // We need to add the sizeof(stack) since in x86(_64) the stack grows
  // downwards.
  .stack = (uintptr_t)stack + sizeof(stack),
  // No flags are currently defined as per spec and should be left to 0.
  .flags = 0,
  // This header structure is the root of the linked list of header tags and
  // points to the first one (and in our case, only).
  .tags = (uintptr_t)&framebuffer_hdr_tag};

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).

// The following will be our kernel's entry point.
extern "C" void _start(Kernel::Stivale::StivaleStruct *stivale2_struct) {
  // Let's get the framebuffer tag.
  Kernel::Stivale::TagFramebuffer *fb_hdr_tag = (Kernel::Stivale::TagFramebuffer *)Kernel::Stivale::GetTag(stivale2_struct, Kernel::Stivale::STRUCT_TAG_FRAMEBUFFER_ID);

  // Check if the tag was actually found.
  if (fb_hdr_tag == NULL) {
    // It wasn't found, just hang...
    for (;;) {
      asm("hlt");
    }
  }

  // Let's get the address of the framebuffer.
  uint8_t *fb_addr = (uint8_t *)fb_hdr_tag->framebuffer_addr;

  // Let's try to paint a few pixels white in the top left, so we know
  // that we booted correctly.
  size_t len = (fb_hdr_tag->framebuffer_bpp / 4) * (fb_hdr_tag->framebuffer_width) * 32;
  for (size_t i = 0; i < len; i++) {
    fb_addr[i] = 0xff;
  }

  // We're done, just hang...
  for (;;) {
    asm("hlt");
  }
}
