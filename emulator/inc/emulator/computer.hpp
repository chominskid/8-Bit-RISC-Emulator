#pragma once

#include <cstdint>
#include <limits>
#include <thread>

#include "memory.hpp"
#include "spinlock.hpp"



class Computer {
private:
    mutable MSSpinLock _state_lock;
    MemoryDevicePointer _memory;
    std::thread _run_thread;
    std::atomic_bool _run;

    struct State {
        uint64_t cycle;
        uint16_t pc;
        uint16_t instruction;
        uint16_t alu_op1; // DECODE -> EXECUTE
        uint16_t result; // EXECUTE -> MEMORY -> WRITE
        uint8_t alu_op2; // DECODE -> EXECUTE
        uint8_t stage;
        uint8_t registers[16];
        uint8_t write_reg; // DECODE -> EXECUTE -> MEMORY -> WRITE
        uint8_t store_val; // DECODE -> EXECUTE -> MEMORY
        uint8_t alu_op; // DECODE -> EXECUTE
        uint8_t mem_op; // DECODE -> EXECUTE -> MEMORY
        bool save_ret; // DECODE
        bool alu_write; // DECODE -> EXECUTE -> MEMORY -> WRITE
        bool alu_set_flags; // DECODE -> EXECUTE
        bool take_jump; // DECODE -> EXECUTE
    };

    State state;

    [[noreturn]] void throw_eil();

    void fetch_stage();
    void decode_alu_op();
    void decode_x_register();
    void decode_y_register();
    void decode_immediate();
    void decode_m_addr_mode();
    void decode_c_addr_mode();
    void decode_jump_condition();
    void decode_stage();
    void execute_stage();
    void memory_stage();
    void writeback_stage();

    void _step();

    void _run_worker(std::chrono::high_resolution_clock::duration period);
    void _step_worker(uint64_t count);
    void _freerun_worker();

public:
    Computer();
    ~Computer();

    void attach_memory(const MemoryDevicePointer& device);

    // reset the computer to its starting state
    void reset();

    // pause execution
    void stop();

    // run the computer for count cycles (default 1)
    void step(uint64_t count = 1);
    // run the computer for count cycles (default 1) in the same thread
    void step_sync(uint64_t count = 1);

    // run the computer at a specified number of cycles per second (default infinity - runs without timer overhead)
    void run(double freq = std::numeric_limits<double>::infinity());

    // fill all memory and registers with zeroes to make debugging easier
    void debug_init();

    // return a string containing the computer's state in a human-readable format
    std::string debug_state() const;
};