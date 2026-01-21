#include "../../inc/emulator/computer.hpp"
#include "../../inc/emulator/spinlock.hpp"

#include <atomic>
#include <bitset>
#include <chrono>
#include <climits>
#include <cmath>
#include <format>
#include <sstream>
#include <stdexcept>
#include <thread>

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
uint8_t sex(uint8_t x) {
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

// TODO: real hardware exceptions
[[noreturn]] void Computer::throw_eil() {
    throw std::runtime_error(std::format("Illegal instruction: {:04x}", state.instruction));
}

void Computer::fetch_stage() {
    const uint8_t high = _memory->read(state.pc).value;
    const uint8_t low = _memory->read(state.pc + 1).value;
    state.instruction = low | high << 8;
    state.pc += 2;
}

void Computer::decode_alu_op() {
    static constexpr std::bitset<16> ALU_WRITE { 0b1011011111001111 };
    static constexpr std::bitset<16> ALU_SETF  { 0b1100011111111111 };
    state.alu_op = (state.instruction & *Encoding::O_MASK) >> *Encoding::O_SHIFT;
    state.alu_write = ALU_WRITE.test(state.alu_op);
    state.alu_set_flags = ALU_SETF.test(state.alu_op);
}

void Computer::decode_x_register() {
    const int idx = (state.instruction & *Encoding::X_MASK) >> *Encoding::X_SHIFT;
    state.alu_op1 = state.registers[idx];
    state.store_val = state.registers[idx];
    state.write_reg = idx;
}

void Computer::decode_y_register() {
    const int idx = (state.instruction & *Encoding::Y_MASK) >> *Encoding::Y_SHIFT;
    state.alu_op2 = state.registers[idx];
}

void Computer::decode_immediate() {
    state.alu_op2 = (state.instruction & *Encoding::IL_MASK) >> *Encoding::IL_SHIFT
            | (state.instruction & *Encoding::IH_MASK) >> *Encoding::IH_SHIFT;
    state.alu_op2 = sex(state.alu_op2);
}

void Computer::decode_m_addr_mode() {
    const int mode = (state.instruction & *Encoding::M_MASK) >> *Encoding::M_SHIFT;
    switch (mode) {
    case *AddrModeM::STACK:
        state.alu_op1 = *AddrModeM::STACK_OFFSET + state.registers[*Register::SP];
        break;
    case *AddrModeM::FRAME:
        state.alu_op1 = *AddrModeM::STACK_OFFSET + state.registers[*Register::FP];
        break;
    case *AddrModeM::REL:
        state.alu_op1 = state.registers[*Register::RA_L] | state.registers[*Register::RA_H] << 8;
        break;
    case *AddrModeM::ZPG:
        state.alu_op1 = state.registers[*Register::GB] + *AddrModeM::ZPG_OFFSET;
        break;
    case *AddrModeM::GE:
        state.alu_op1 = state.registers[*Register::GE_L] | state.registers[*Register::GE_H] << 8;
        break;
    case *AddrModeM::GF:
        state.alu_op1 = state.registers[*Register::GF_L] | state.registers[*Register::GF_H] << 8;
        break;
    case *AddrModeM::GG:
        state.alu_op1 = state.registers[*Register::GG_L] | state.registers[*Register::GG_H] << 8;
        break;
    case *AddrModeM::GH:
        state.alu_op1 = state.registers[*Register::GH_L] | state.registers[*Register::GH_H] << 8;
        break;
    default:
        throw_eil();
    }
}

void Computer::decode_c_addr_mode() {
    const int mode = (state.instruction & *Encoding::M_MASK) >> *Encoding::M_SHIFT;
    switch (mode) {
    case *AddrModeC::BLD_LOW:
        state.alu_op1 = *AddrModeC::BLD_LOW_OFFSET;
        break;
    case *AddrModeC::BLD_HIGH:
        state.alu_op1 = *AddrModeC::BLD_HIGH_OFFSET;
        break;
    case *AddrModeC::REL:
        state.alu_op1 = state.pc;
        break;
    case *AddrModeC::RET:
        state.alu_op1 = state.registers[*Register::RA_L] | state.registers[*Register::RA_H] << 8;
        break;
    case *AddrModeC::GE:
        state.alu_op1 = state.registers[*Register::GE_L] | state.registers[*Register::GE_H] << 8;
        break;
    case *AddrModeC::GF:
        state.alu_op1 = state.registers[*Register::GF_L] | state.registers[*Register::GF_H] << 8;
        break;
    case *AddrModeC::GG:
        state.alu_op1 = state.registers[*Register::GG_L] | state.registers[*Register::GG_H] << 8;
        break;
    case *AddrModeC::GH:
        state.alu_op1 = state.registers[*Register::GH_L] | state.registers[*Register::GH_H] << 8;
        break;
    default:
        throw_eil();
    }
}

void Computer::decode_jump_condition() {
    const bool c = get_bit(state.registers[*Register::SR], *Status::C_SHIFT);
    const bool v = get_bit(state.registers[*Register::SR], *Status::V_SHIFT);
    const bool n = get_bit(state.registers[*Register::SR], *Status::N_SHIFT);
    const bool z = get_bit(state.registers[*Register::SR], *Status::Z_SHIFT);

    switch ((state.instruction & *Encoding::C_MASK) >> *Encoding::C_SHIFT) {
    case *JumpCond::C: state.take_jump = c; break;
    case *JumpCond::V: state.take_jump = v; break;
    case *JumpCond::N: state.take_jump = n; break;
    case *JumpCond::Z: state.take_jump = z; break;
    case *JumpCond::G: state.take_jump = (!v ? !n : c) && !z; break;
    case *JumpCond::GE: state.take_jump = !v ? !n : c; break;
    case *JumpCond::GU: state.take_jump = c && !z; break;
    case *JumpCond::ALW: state.take_jump = true; break;
    default:
        throw_eil();
    }
    
    if (state.instruction & *Encoding::N_MASK)
        state.take_jump = !state.take_jump;
}

void Computer::decode_stage() {
    state.take_jump = false;
    state.alu_set_flags = true;
    state.save_ret = false;
    state.mem_op = *MemOp::NONE;
    switch ((state.instruction & *Encoding::FMT_MASK) >> *Encoding::FMT_SHIFT) {
    case *Encoding::FMT_A:
        decode_alu_op();
        decode_x_register();
        decode_y_register();
        break;
    case *Encoding::FMT_IA:
        decode_alu_op();
        decode_x_register();
        decode_immediate();
        break;
    case *Encoding::FMT_M:
        decode_x_register();
        decode_m_addr_mode();
        decode_immediate();
        state.alu_op = *ALUOp::ADD;
        state.alu_write = !(state.instruction & *Encoding::S_MASK);
        state.alu_set_flags = false;
        state.mem_op = (state.instruction & *Encoding::S_MASK) >> *Encoding::S_SHIFT;
        break;
    case *Encoding::FMT_C:
        decode_c_addr_mode();
        decode_jump_condition();
        decode_immediate();
        state.alu_op2 <<= 1;
        state.alu_op = *ALUOp::ADD;
        state.alu_write = false;
        state.alu_set_flags = false;
        state.save_ret = (state.instruction & *Encoding::S_MASK) >> *Encoding::S_SHIFT;
        break;
    default:
        throw_eil();
    }
}

void Computer::execute_stage() {
    uint8_t sr = state.registers[*Register::SR];

    switch (state.alu_op) { // set default carry state
    case *ALUOp::ADD:
        set_bit(sr, *Status::C_SHIFT, false);
        break;
    case *ALUOp::SUB:
    case *ALUOp::CMP:
        set_bit(sr, *Status::C_SHIFT, true);
    default:
        break;
    }

    switch (state.alu_op) { // invert operand for subtraction
    case *ALUOp::SUB:
    case *ALUOp::CMP:
    case *ALUOp::SBC:
    case *ALUOp::CMC:
        state.alu_op2 = ~state.alu_op2;
    default:
        break;
    }

    uint16_t res = 0;
    switch (state.alu_op) {
    case *ALUOp::ADD:
    case *ALUOp::ADC:
    case *ALUOp::SUB:
    case *ALUOp::CMP:
    case *ALUOp::SBC:
    case *ALUOp::CMC:
        res = (state.alu_op1 & 0x00FF) + state.alu_op2 + get_bit(sr, *Status::C_SHIFT);
        set_bit(sr, *Status::C_SHIFT, res & 0x0100);
        set_bit(sr, *Status::V_SHIFT, (state.alu_op1 & 0x0080) == (state.alu_op2 & 0x0080) && (state.alu_op1 & 0x0080) != (res & 0x0080));
        res += state.alu_op1 & 0xFF00;
        if (state.alu_op2 & 0x0080)
            res += 0xFF00;
        break;
    case *ALUOp::AND:
        res = state.alu_op1 & state.alu_op2;
        break;
    case *ALUOp::OR:
        res = state.alu_op1 | state.alu_op2;
        break;
    case *ALUOp::XOR:
        res = state.alu_op1 ^ state.alu_op2;
        break;
    case *ALUOp::SHL:
        res = (state.alu_op1 & 0xFF00) | ((state.alu_op1 << (state.alu_op2 & 0x0007)) & 0x00FF);
        break;
    case *ALUOp::SHR:
        res = (state.alu_op1 & 0xFF00) | ((state.alu_op1 >> (state.alu_op2 & 0x0007)) & 0x00FF);
        break;
    case *ALUOp::MOV:
        res = state.alu_op2;
        break;
    case *ALUOp::MOVH:
        res = (state.alu_op1 & 0x003F) | ((state.alu_op2 << 6) & 0x00C0);
        break;
    case *ALUOp::SEB:
        res = state.alu_op1;
        set_bit(res, state.alu_op2 & 0x0007, state.alu_op2 & 0x0008);
        break;
    case *ALUOp::TSB:
        break;
    default:
        throw_eil();
    }

    switch (state.alu_op) { // set z flag
    case *ALUOp::TSB:
        set_bit(sr, *Status::Z_SHIFT, get_bit(state.alu_op1, state.alu_op2 & 0x0007));
        break;
    case *ALUOp::ADC:
    case *ALUOp::SBC:
    case *ALUOp::CMC:
        and_bit(sr, *Status::Z_SHIFT, (res & 0x00FF) == 0);
        break;
    default:
        set_bit(sr, *Status::Z_SHIFT, (res & 0x00FF) == 0);
        break;
    }

    switch (state.alu_op) { // set n flag
    case *ALUOp::TSB:
        set_bit(sr, *Status::N_SHIFT, get_bit(state.alu_op2, state.alu_op2 & 0x0038));
        break;
    default:
        set_bit(sr, *Status::N_SHIFT, res & 0x0080);
        break;
    }

    state.result = res;

    if (state.alu_set_flags)
        state.registers[*Register::SR] = sr;

    if (state.save_ret) {
        state.registers[*Register::RA_L] = state.pc;
        state.registers[*Register::RA_H] = state.pc >> 8;
    }

    if (state.take_jump)
        state.pc = res;
}

void Computer::memory_stage() {
    switch (state.mem_op) {
    case *MemOp::LOAD:
        state.result = _memory->read(state.result).value;
        break;
    case *MemOp::STORE:
        _memory->write(state.result, state.store_val);
        break;
    default:
        break;
    }
}

void Computer::writeback_stage() {
    if (state.alu_write)
        state.registers[state.write_reg] = state.result;
}

Computer::Computer() :
    _run(false) 
{}

Computer::~Computer() {
    stop();
}

void Computer::attach_memory(const MemoryDevicePointer& device) {
    _memory = device;
}

void Computer::reset() {
    MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::SLAVE);
    state.stage = 0;
    state.cycle = 0;
    state.pc = 0x0000;
    state.registers[*Register::SR] = 0;
}

void Computer::_step() {
    switch (state.stage++) {
    case 0: fetch_stage(); break;
    case 1: decode_stage(); break;
    case 2: execute_stage(); break;
    case 3: memory_stage(); break;
    case 4: writeback_stage();
        state.stage = 0;
        break;
    }

    ++state.cycle;
    if (state.cycle == 0)
        throw std::runtime_error("You rolled over the cycle counter. How?");
}

static constexpr unsigned int MAX_FREERUN = 1000000;

void Computer::_run_worker(std::chrono::high_resolution_clock::duration period) {
    using namespace std::chrono_literals;
    auto then = std::chrono::high_resolution_clock::now();
    while (_run.load(std::memory_order_relaxed)) {
        auto next = then + period;
        MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::SLAVE);
        unsigned int c = 0;
        auto now = std::chrono::high_resolution_clock::now();
        while (next <= now && c != MAX_FREERUN) {
            then = next;
            _step();
            next += period;
            ++c;
        }
        guard.release();
        if (c < MAX_FREERUN)
            std::this_thread::sleep_for(1ms);
    }
}

void Computer::_step_worker(uint64_t count) {
    while (_run.load(std::memory_order_relaxed)) {
        MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::SLAVE);
        for (unsigned int c = 0; c != MAX_FREERUN; ++c) {
            if (count == 0)
                return;
            --count;
            _step();
        }
    }
}

void Computer::_freerun_worker() {
    while (_run.load(std::memory_order_relaxed)) {
        MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::SLAVE);
        // for (unsigned int c = 0; c != MAX_FREERUN; ++c)
        //     _step();
        unsigned int c = 0;
        while (c != MAX_FREERUN) {
            _step();
            ++c;
        }
    }
}

void Computer::stop() {
    _run.store(false, std::memory_order_relaxed);
    if (_run_thread.joinable())
        _run_thread.join();
}

void Computer::step(uint64_t count) {
    stop();
    _run.store(true, std::memory_order_relaxed);
    _run_thread = std::thread(&Computer::_step_worker, this, count);
}

void Computer::step_sync(uint64_t count) {
    stop();
    _run.store(true, std::memory_order_relaxed);
    _step_worker(count);
}

void Computer::run(double freq) {
    stop();
    _run.store(true, std::memory_order_relaxed);
    if (std::isfinite(freq))
        _run_thread = std::thread(&Computer::_run_worker, this, std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<double>(1.0 / freq)));
    else
        _run_thread = std::thread(&Computer::_freerun_worker, this);
}

// Print a human-readable number (in hex, binary and unsigned and signed decimal).
template <std::integral T>
std::string hr_num(T x) {
    using S = std::make_signed_t<T>;
    using U = std::make_unsigned_t<T>;
    constexpr size_t bin_width = std::numeric_limits<U>::digits;
    constexpr size_t hex_width = bin_width / 4;
    return std::format("x{0:0{2}x} b{0:0{3}b} {0:d} {1:+d}", (U)x, (S)x, hex_width, bin_width);
}

// Print human-readable number (in hex and binary).
template <std::integral T>
std::string hr_data(T x) {
    using U = std::make_unsigned_t<T>;
    constexpr size_t bin_width = std::numeric_limits<U>::digits;
    constexpr size_t hex_width = bin_width / 4;
    return std::format("x{0:0{1}x} b{0:0{2}b}", (U)x, hex_width, bin_width);
}

std::string hr_freq(double freq) {
    const double l = std::log10(freq);

    if (l >= 9.0)
        return std::format("{:.2f} GHz", freq / 1000000000.0);
    else if (l >= 6.0)
        return std::format("{:.2f} MHz", freq / 1000000.0);
    else if (l >= 3.0)
        return std::format("{:.2f} kHz", freq / 1000.0);
    else
        return std::format("{:.2f} Hz", freq);
}

void Computer::debug_init() {
    MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::MASTER);
    state.instruction = 0;
    state.result = 0;
    state.alu_op1 = 0;
    std::fill(state.registers, state.registers + 16, 0);
    _memory->debug_fill(_memory->size(), 0);
    state.write_reg = 0;
    state.store_val = 0;
    state.alu_op2 = 0;
    state.alu_op = 0;
    state.mem_op = 0;
    state.save_ret = false;
    state.alu_write = false;
    state.alu_set_flags = false;
    state.take_jump = false;
}

std::string Computer::debug_state() const {
    static auto then = std::chrono::high_resolution_clock::now();
    static uint64_t cycle_then = 0;
    static double freq = 0.0;
    
    MSSpinLockGuard guard(_state_lock, MSSpinLockGuard::Type::MASTER);
    const auto now = std::chrono::high_resolution_clock::now();
    const State copy = state;
    guard.release();

    const double dt = std::chrono::duration<double>(now - then).count();    
    const uint64_t diff = copy.cycle - cycle_then;
    if (dt >= 0.5 && (diff >= 10 || dt >= 1.0)) {
        freq = diff / dt;
        then = now;
        cycle_then += diff;
    }

    std::stringstream s;
    s << "freq:  " << hr_freq(freq) << '\n';
    std::string stage_str = " F  D  X  M  W ";
    stage_str[3 * copy.stage] = '[';
    stage_str[3 * copy.stage + 2] = ']';
    s << "cycle: " << std::format("{:d}", copy.cycle) << '\n';
    s << "stage: " << stage_str << '\n';
    s << "pc:    " << hr_num(copy.pc) << '\n';
    s << "inst:  " << hr_data(copy.instruction) << '\n';
    s << "op1:   " << hr_num(copy.alu_op1) << '\n';
    s << "op2:   " << hr_num(copy.alu_op2) << '\n';
    s << "res:   " << hr_num(copy.result) << '\n';
    s << "jump:  " << std::format("{:s}", copy.take_jump) << '\n';
    s << "sret:  " << std::format("{:s}", copy.save_ret) << '\n';
    s << "sres:  " << std::format("{:s}", copy.alu_write) << '\n';
    s << "setf:  " << std::format("{:s}", copy.alu_set_flags) << '\n';
    s << "store: " << hr_num(copy.store_val) << '\n';
    s << '\n';
    s << "ra:    " << hr_num(bytes_to_num<uint16_t>(&copy.registers[*Register::RA_L])) << '\n';
    s << "ra.l:  " << hr_num(copy.registers[*Register::RA_L]) << '\n';
    s << "ra.h:  " << hr_num(copy.registers[*Register::RA_H]) << '\n';
    s << "sr:    " << hr_data(copy.registers[*Register::SR]) << '\n';
    s << "sp:    " << hr_num(copy.registers[*Register::SP]) << '\n';
    s << "fp/ga: " << hr_num(copy.registers[*Register::FP]) << '\n';
    s << "gb:    " << hr_num(copy.registers[*Register::GB]) << '\n';
    s << "gc:    " << hr_num(copy.registers[*Register::GC]) << '\n';
    s << "gd:    " << hr_num(copy.registers[*Register::GD]) << '\n';
    s << "ge:    " << hr_num(bytes_to_num<uint16_t>(&copy.registers[*Register::GE_L])) << '\n';
    s << "ge.l:  " << hr_num(copy.registers[*Register::GE_L]) << '\n';
    s << "ge.h:  " << hr_num(copy.registers[*Register::GE_H]) << '\n';
    s << "gf:    " << hr_num(bytes_to_num<uint16_t>(&copy.registers[*Register::GF_L])) << '\n';
    s << "gf.l:  " << hr_num(copy.registers[*Register::GF_L]) << '\n';
    s << "gf.h:  " << hr_num(copy.registers[*Register::GF_H]) << '\n';
    s << "gg:    " << hr_num(bytes_to_num<uint16_t>(&copy.registers[*Register::GG_L])) << '\n';
    s << "gg.l:  " << hr_num(copy.registers[*Register::GG_L]) << '\n';
    s << "gg.h:  " << hr_num(copy.registers[*Register::GG_H]) << '\n';
    s << "gh:    " << hr_num(bytes_to_num<uint16_t>(&copy.registers[*Register::GH_L])) << '\n';
    s << "gh.l:  " << hr_num(copy.registers[*Register::GH_L]) << '\n';
    s << "gh.h:  " << hr_num(copy.registers[*Register::GH_H]) << '\n';
    return s.str();
}