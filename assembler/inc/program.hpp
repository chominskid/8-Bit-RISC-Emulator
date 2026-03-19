#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/inc/memorymap.hpp"
#include "parser.hpp"
#include "instruction.hpp"

class Program {
private:
    struct Placeholder {
        std::optional<size_t> fixed_address;
        size_t tentative_address;
        size_t tentative_encoding;
        const Instruction* instruction;
        Origin origin;
        std::vector<TokenPtr> args;
        std::vector<std::string> failures;
        Encoder::Result last_output;
        bool final;

        Placeholder(std::optional<size_t> fixed_address, const std::vector<uint8_t>& data, Origin origin);
        Placeholder(std::optional<size_t> fixed_address, const Instruction& instruction, std::vector<TokenPtr>&& args, Origin origin);
        Placeholder(Placeholder&& other);

        bool try_encode();

        void throw_failure();
        size_t tentative_size() const;
    };

    std::unordered_map<std::string, size_t> labels;
    std::vector<Placeholder> program;
    std::optional<size_t> next_fixed_address;

    bool try_assemble_pass();

public:
    Program();

    void move(size_t address);
    void add_data(const std::vector<uint8_t>& data, Origin origin);
    void add_instruction(std::vector<std::unique_ptr<Token>>&& args);
    void add_label(std::string&& value, Origin origin);

    MemoryMap assemble();
};