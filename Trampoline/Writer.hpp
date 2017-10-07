#ifndef TRAMPOLINE_WRITER_HPP
#define TRAMPOLINE_WRITER_HPP

#include <array>

struct Writer {
public:
    Writer(char*& in_ptr) : ptr(in_ptr) {}

    // Adding instructions by 1 byte
    template<size_t N> void add1(const std::array<char, N> &operation) {
        for (size_t i = 0; i < operation.size(); ++i) {
            *(ptr++) = operation[i];
        }
    }

    // Adding instructions by 1 byte and also 4 bytes of data
    template<size_t N> void add4(const std::array<char, N> &operation, int32_t data) {
        add1(operation);
        *(int32_t *) ptr = data;
        ptr += 4;
    }

    // Adding instructions by 1 byte and also 8 bytes of data
    template<size_t N> void add8(const std::array<char, N> &operation, void *data) {
        add1(operation);
        *(void **) ptr = data;
        ptr += 8;
    }

    // Skip space for a num value
    void *reserve(size_t num) {
        void *start = ptr;
        ptr += num;
        return start;
    }

    // Return current pointer position
    void *get_ptr() {
        return ptr;
    }
private:
    char* &ptr;
};

#endif //TRAMPOLINE_WRITER_HPP
