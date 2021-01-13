#include <Kernel/Stivale.hpp>
#include <stddef.h>
#include <stdint.h>

Kernel::Stivale::Tag *Kernel::Stivale::GetTag(struct Kernel::Stivale::StivaleStruct *stivale2_struct, uint64_t id) {
  Kernel::Stivale::Tag *current_tag = (Kernel::Stivale::Tag *)stivale2_struct->tags;
  for (;;) {
    // If the tag pointer is NULL (end of linked list), we did not find
    // the tag. Return NULL to signal this.
    if (current_tag == NULL) {
      return NULL;
    }

    // Check whether the identifier matches. If it does, return a pointer
    // to the matching tag.
    if (current_tag->identifier == id) {
      return current_tag;
    }

    // Get a pointer to the next tag in the linked list and repeat.
    current_tag = (Kernel::Stivale::Tag *)current_tag->next;
  }
}