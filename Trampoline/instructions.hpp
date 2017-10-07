#ifndef TRAMPOLINE_INSTRUCTIONS_HPP
#define TRAMPOLINE_INSTRUCTIONS_HPP

#include <array>
namespace instruction {
    const std::array<char, 1> je_imm  = {'\x74'};
    const std::array<char, 1> jmp_imm = {'\xEB'};
    const std::array<char, 1> ret     = {'\xC3'};

    const std::array<char, 2> push_r9     = {'\x41', '\x51'};
    const std::array<char, 2> jmp_rax     = {'\xFF', '\xE0'};
    const std::array<char, 2> mov_rdi_imm = {'\x48', '\xBF'};
    const std::array<char, 2> mov_rax_imm = {'\x48', '\xB8'};
    const std::array<char, 2> add_rax_imm = {'\x48', '\x05'};
    const std::array<char, 2> call_rax    = {'\xFF', '\xD0'};
    const std::array<char, 2> pop_r9      = {'\x41', '\x59'};

    const std::array<char, 3> add_rsp_imm = {'\x48', '\x81', '\xC4'};
    const std::array<char, 3> cmp_rax_rsp = {'\x48', '\x39', '\xE0'};
    const std::array<char, 3> mov_rax_rsp = {'\x48', '\x89', '\xE0'};
    const std::array<char, 3> sub_rsp_imm = {'\x48', '\x81', '\xEC'};

    const std::array<char, 4> mov_val_r11_rsp       = {'\x4C', '\x8B', '\x1C', '\x24'}; // r11   <- [rsp]
    const std::array<char, 4> mov_val_rdi_rsp       = {'\x48', '\x8B', '\x3C', '\x24'}; // rdi   <- [rsp]
    const std::array<char, 4> mov_val_rsp_r11       = {'\x4C', '\x89', '\x1C', '\x24'}; // [rsp] <- r11
    const std::array<char, 4> mov_val_r11_rsp_imm   = {'\x4C', '\x8B', '\x9C', '\x24'}; // [r11] <- [rsp+imm]

    const std::array<char, 5> mov_val_rsp_rdi = {'\x48', '\x89', '\x7C', '\x24', '\xF8'};
}
#endif //TRAMPOLINE_INSTRUCTIONS_HPP
