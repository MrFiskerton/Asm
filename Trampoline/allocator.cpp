#include "allocator.hpp"

namespace {
    const size_t NUM_PAGES = 1;
    const size_t PAGE_SIZE = (1 << 12);      //4096
    const size_t TRAMPOLINE_SIZE = (1 << 8); //256
    void *allocated; //Points to the beginning of the mapped area
    void *head;      //Points to first free node in a list

    void alloc_mem() {
        allocated = mmap(
                nullptr,
                PAGE_SIZE * NUM_PAGES,
                PROT_EXEC | PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1, 0
        );

        head = (void **) allocated;
        if (allocated != nullptr) {
            //Link list
            for (size_t i = 0; i < PAGE_SIZE * NUM_PAGES; i += TRAMPOLINE_SIZE) {
                auto prev = ((char *) allocated) + i;
                *(void **) prev = 0;
                if (i) { *(void **) (prev - TRAMPOLINE_SIZE) = prev; }
            }
        }
    }
}

void *allocator::alloc() {
    if (head == nullptr) {
        alloc_mem();
        if (head == nullptr) { return nullptr; }
    }
    //Move head forward
    void *result = head;
    head = *(void **) head;
    return result;
}

void allocator::free(void *del) {
    //Move head backwards
    *(void **) del = head;
    head = (void **) del;
}

void allocator::unmap() {
    munmap(allocated, PAGE_SIZE * NUM_PAGES);
}