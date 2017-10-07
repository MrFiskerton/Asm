#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include "argument_types.hpp"
#include "allocator.hpp"
#include "instructions.hpp"

#include <iostream>
#include <memory>
#include <unistd.h>

template<typename Return>
struct Trampoline;

template<typename Return, typename ... Args>
struct Trampoline<Return(Args ...)> {
public:
    template<typename F> Trampoline(F func)
            : func_obj(new F(std::move(func))),
              deleter(my_deleter<F>),
              code(allocator::alloc()),
              ptr((char *) code)
    {
        if (arg_type<Args ...>::reg < 6) {
            mov_regs(arg_type<Args ...>::reg - 1, 0);                // Shift arguments by 1 position
            add8(instruction::mov_rdi_imm, func_obj);                // Put funtion object to RDI as first argument
            add8(instruction::mov_rax_imm, (void *) &do_call<F>);    // Put pointer to caller in RAX
            add1(instruction::jmp_rax);                              // Call for the function
        } else {
            /*
                1. Store last argument
                2. Move in cycle integer arguments
                3. Set up return address in right position
                4. Move in cycle arguments in stack
                5. Set up last argument from prev. regs
                6. Prepare stack to call and call function object
                7. Make up stack due to calling-convs.
            */

            add1(instruction::mov_val_r11_rsp);  // Saving return address from top of stack
            mov_regs(5, 0);                      // Move all int arguments by 1 (last on stack)
            add1(instruction::mov_rax_rsp);      // Store current top of stack to rax

            // Count the required size of stack:
            int stack_size = 8 * (arg_type<Args ...>::reg - 6 + std::max(arg_type<Args ...>::sse - 8, 0));

            add4(instruction::add_rax_imm, stack_size + 8);  // Put rax as last argument in stack
            add4(instruction::add_rsp_imm, 8);               // Put rsp as return address

            //-------Making loop to shift arguments in stack-----------//
            char *label = (char *) get_ptr();   // Creating a label for loop
            add1(instruction::cmp_rax_rsp);     // Checking if arguments're copied
            add1(instruction::je_imm);          // JumpEqual condition after comparation
            char *label2 = (char *) reserve(1); // 1 byte for label address

            // Get value of next argument:
            add4(instruction::add_rsp_imm, 8);  // Move stack pointer
            add1(instruction::mov_val_rdi_rsp); // Copy value to rdi
            add1(instruction::mov_val_rsp_rdi); // Put from rdi to rsp-0x8

            // Storing relative addresses of labels:
            add1(instruction::jmp_imm);                         // Jump to first label
            char *label3 = (char *) reserve(1);                 // 1 byte for label address
            *label3 = (char) (label - (char *) get_ptr());      // Store in reserved space offset from label
            *label2 = (char) ((char *) get_ptr() - label2 - 1); // Store in reserved place offset from label 2
            //----------------------Loop ended-------------------------//

            add1(instruction::mov_val_rsp_r11);                              // Set up saved return address to stack
            add4(instruction::sub_rsp_imm, stack_size + 8);                  // Transfer rsp to top of stack
            add8(instruction::mov_rdi_imm, func_obj);                        // Put function object to rdi
            add8(instruction::mov_rax_imm, (void *) &do_call < F > );        // Put call address to rax

            add1(instruction::call_rax);                                     // Calling function

            // Turning stack to pre-call condition:
            add1(instruction::pop_r9);                                       // Removing 6th argument from stack
            add4(instruction::mov_val_r11_rsp_imm, stack_size);              // Normalize stack size
            add1(instruction::mov_val_rsp_r11);                              // Restore rsp value
            add1(instruction::ret);                                          // Return - end function call
        }
    }

    ~Trampoline() {
        if (func_obj) {deleter(func_obj);}
        allocator::free(code);
    }

    template<typename F> static Return do_call(void *obj, Args ... args) {
        return (*(F *) obj)(args ...);
    }

    Return (*get() const )(Args ... args) {
        return (Return (*)(Args ... args)) code;
    }

private:
    void *func_obj;
    void (*deleter)(void *);
    void *code;
    char *ptr;
private:
    template<typename F> static void my_deleter(void *func_obj) {
        delete static_cast <F *> (func_obj);
    }

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

    // Moving default arguments registers
    void mov_regs(int from, int to) {
        static const char *shifts[] = {
                "\x48\x89\xFE", // rdi -> rsi
                "\x48\x89\xF2", // rsi -> rdx
                "\x48\x89\xD1", // rdx -> rcx
                "\x49\x89\xC8", // rcx -> r8
                "\x4D\x89\xC1", // r8 -> r9
                "\x41\x51"      // push r9
        };

        for (int i = from; i >= to; i--) {
            // WARNING: This is a weak place for C strings
            for (const char *j = shifts[i]; *j; j++) {
                *(ptr++) = *j;
            }
        }
    }
};

//------------------------READ ME--------------------------//
/* Base trampoline:
    trampoline:
        mov rdi -> rsi  // shift argument by 1
        mov imm -> rdi  // put pointer to object
        mov imm -> rax  // put pointer to call instruction
        jmp rax         // do call
    do_call:
        push rbp
        mov rsp -> rbp
        sub 0x10 from rsp
        mov rdi -> [rbp - 0x8]
        mov esi -> [rbp - 0xc]
        mov [rbp - 0xc] -> edx
        mov [rbp - 0x8] -> rax
        mov edi -> esi
        mov rax -> rdi
        call head
        ret                                                */
//---------------------------END---------------------------//
#endif // TRAMPOLINE_H