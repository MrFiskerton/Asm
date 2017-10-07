#ifndef TRAMPOLINE_ALLOCATOR_H
#define TRAMPOLINE_ALLOCATOR_H

#include <stdlib.h>
#include <sys/mman.h>

namespace allocator {
    /**
     * Allocate memory for fucntion
     */
    void *alloc();

    /**
     * Clear space of function
     */
    void free(void *);

    /**
     * Unmap using memory
     */
    void unmap();
}

#endif // TRAMPOLINE_ALLOCATOR_H