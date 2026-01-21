#pragma once

#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

static_assert(CHAR_BIT == 8);

template <typename E>
requires std::is_enum_v<E>
constexpr auto operator*(E x) {
    return static_cast<std::underlying_type_t<E>>(x);
}

template <std::integral T>
constexpr bool get_bit(const T& x, int n) {
    return x & ((T)1 << n);
}

template <std::integral T>
constexpr void set_bit(T& x, int n, bool state) {
    if (state) x |= (T)1 << n;
    else       x &= ~((T)1 << n);
}

template <std::integral T>
constexpr void and_bit(T& x, int n, bool state) {
    if (!state) x &= ~((T)1 << n);
}

// sign-extend x from 6 bits to 8 bits
inline uint8_t sex(uint8_t x) {
    if (x & 0x20)
        x |= 0xC0;
    return x;
}

template <std::integral T>
T bytes_to_num(const void* bytes) {
    using U = std::make_unsigned_t<T>;
    U x = 0;
    const uint8_t* _bytes = reinterpret_cast<const uint8_t*>(bytes);
    for (size_t i = 0; i < sizeof(U); ++i)
        x |= (U)_bytes[i] << (i * 8);
    return x;
}

enum class Register {
    RA_L,
    RA_H,
    SR,
    SP,
    FP,
    GA = FP,
    GB,
    GC,
    GD,
    GE_L,
    GE_H,
    GF_L,
    GF_H,
    GG_L,
    GG_H,
    GH_L,
    GH_H
};

enum class ALUOp {
    ADD,
    ADC,
    SUB,
    SBC,
    CMP,
    CMC,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    _,
    MOV,
    MOVH,
    TSB,
    SEB,
};

enum class MemOp {
    LOAD,
    STORE,
    NONE,
};

enum class Status : uint8_t {
    C_MASK = 0x08,
    V_MASK = 0x04,
    N_MASK = 0x02,
    Z_MASK = 0x01,

    C_SHIFT = 3,
    V_SHIFT = 2,
    N_SHIFT = 1,
    Z_SHIFT = 0,
};

enum class AddrModeM : uint16_t {
    STACK = 0x00,
    FRAME = 0x01,
    REL = 0x02,
    ZPG = 0x03,
    GE = 0x04,
    GF = 0x05,
    GG = 0x06,
    GH = 0x07,

    STACK_OFFSET = 0x0100,
    ZPG_OFFSET = 0x0200,
};

enum class AddrModeC : uint16_t {
    BLD_LOW = 0x00,
    BLD_HIGH = 0x01,
    REL = 0x02,
    RET = 0x03,
    GE = 0x04,
    GF = 0x05,
    GG = 0x06,
    GH = 0x07,

    BLD_LOW_OFFSET = 0x0040,
    BLD_HIGH_OFFSET = 0x00C0,
};

enum class JumpCond {
    C,
    V,
    N,
    Z,
    G,
    GE,
    GU,
    ALW,
};

enum class Encoding : uint16_t {
    FMT_A = 0x00,
    FMT_IA = 0x01,
    FMT_M = 0x02,
    FMT_C = 0x03,

    X_SHIFT = 4,
    X_MASK = 0x00F0,

    Y_SHIFT = 0,
    Y_MASK = 0x000F,

    O_SHIFT = 10,
    O_MASK = 0x3C00,

    IL_SHIFT = 0,
    IL_MASK = 0x000F,

    IH_SHIFT = 4,
    IH_MASK = 0x0300,

    M_SHIFT = 10,
    M_MASK = 0x1C00,

    S_SHIFT = 13,
    S_MASK = 0x2000,

    C_SHIFT = 4,
    C_MASK = 0x0070,

    N_SHIFT = 7,
    N_MASK = 0x0080,

    FMT_SHIFT = 14,
    FMT_MASK = 0xC000,
};