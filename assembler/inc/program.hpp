#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser.hpp"
#include "instruction.hpp"

class Program {
private:
    struct Placeholder {
        std::optional<size_t> fixed_address;
        size_t tentative_address;
        size_t tentative_encoding;
        const Instruction& instruction;
        std::vector<TokenPtr> args;
        std::vector<std::string> failures;
        Encoder::Result last_output;
        bool final;

        Placeholder(std::optional<size_t> fixed_address, const Instruction& instruction, std::vector<TokenPtr>&& args);
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

    void add_instruction(std::vector<std::unique_ptr<Token>>&& args);
    void add_label(std::string&& value);

    std::vector<uint8_t> assemble();
};