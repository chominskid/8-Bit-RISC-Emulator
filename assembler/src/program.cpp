#include "../inc/program.hpp"
#include "../inc/error.hpp"
#include <format>
#include <sstream>

Program::Placeholder::Placeholder(std::optional<size_t> fixed_address, const Instruction& instruction, std::vector<TokenPtr>&& args):
    fixed_address(fixed_address),
    tentative_encoding(0),
    instruction(instruction),
    args(std::forward<std::vector<TokenPtr>>(args)),
    final(false)
{
    if (!instruction.independent)
        return;

    for (; tentative_encoding < instruction.encoders.size(); ++tentative_encoding) {
        Encoder::Result result = instruction.encoders[tentative_encoding].encode(0, this->args);

        if (!result.has_value()) {
            failures.emplace_back(std::move(result.error()));
            continue;
        }

        last_output = std::move(result);
        final = true;
        break;
    }

    if (tentative_encoding == instruction.encoders.size())
        throw_failure();
}

Program::Placeholder::Placeholder(Placeholder&& other) :
    fixed_address(other.fixed_address),
    tentative_address(other.tentative_address),
    tentative_encoding(other.tentative_encoding),
    instruction(other.instruction),
    args(std::move(other.args)),
    failures(std::move(other.failures)),
    last_output(std::move(other.last_output)),
    final(other.final)
{}

bool Program::Placeholder::try_encode() {
    Encoder::Result result = instruction.encoders[tentative_encoding].encode(tentative_address, args);
    if (!result.has_value()) {
        failures.emplace_back(std::move(result.error()));
        ++tentative_encoding;
        if (tentative_encoding == instruction.encoders.size())
            throw_failure();
        return false;
    } else {
        last_output = std::move(result);
        return true;
    }
}

void Program::Placeholder::throw_failure() {
    std::stringstream s;
    s << "Could not encode instruction " << instruction.signature.to_string() << ":\n";
    for (size_t i = 0; i < failures.size(); ++i)
        s << "  Encoding " << i << ": " << failures[i] << '\n';
    throw AssemblerError(s.str());
}

size_t Program::Placeholder::tentative_size() const {
    if (final)
        return last_output.size();
    else if (instruction.encoders[tentative_encoding].size)
        return *instruction.encoders[tentative_encoding].size;
    throw AssemblerError("Could not get tentative size of variable-size instruction encoding.");
}

Program::Program() :
    next_fixed_address(0)
{}

void Program::add_instruction(std::vector<TokenPtr>&& args) {
    const Opcode::Value opcode = args[0]->get<Opcode>().value;
    args.erase(args.begin());
    Signature signature(opcode, args);
    auto it = INSTRUCTIONS.find(signature);
    if (it == INSTRUCTIONS.end())
        throw AssemblerError(std::format("Unknown instruction {}", signature.to_string()));
    program.emplace_back(
        next_fixed_address,
        *it,
        std::forward<std::vector<TokenPtr>>(args)
    );
    next_fixed_address.reset();

    if (program.back().fixed_address)
        program.back().tentative_address = *program.back().fixed_address;
}

void Program::add_label(std::string&& value) {
    auto res = labels.emplace(std::forward<std::string>(value), program.size());
    if (!res.second)
        throw AssemblerError(std::format("Label {} already exists.", res.first->first));
}

bool Program::try_assemble_pass() {
    size_t address = 0;
    for (size_t i = 0; i < program.size(); ++i) {
        if (program[i].fixed_address) {
            address = *program[i].fixed_address + program[i].tentative_size();
            continue;
        }

        program[i].tentative_address = address;
        address += program[i].tentative_size();
    }

    for (size_t i = 0; i < program.size(); ++i) {
        if (program[i].final)
            continue;

        for (const TokenPtr& arg: program[i].args) {
            if (arg->type() != Token::Type::LABEL)
                continue;
            LabelArg& label = arg->get<LabelArg>();
            auto it = labels.find(label.value);
            if (it == labels.end())
                throw AssemblerError(std::format("Label {} is not defined.", label.value));

            if (it->second == program.size()) {
                label.address = address;
            } else {
                label.address = program[it->second].tentative_address;
            }
        }

        if (!program[i].try_encode())
            return false;
    }

    return true;
}

std::vector<uint8_t> Program::assemble() {
    uint64_t pass_count = 0;
    for (;;) {
        if (try_assemble_pass())
            break;
    }

    std::vector<uint8_t> output;
    for (size_t i = 0; i < program.size(); ++i) {
        const size_t req_size = program[i].tentative_address + program[i].last_output.size();
        if (output.size() < req_size)
            output.resize(req_size);
        std::copy(program[i].last_output.begin(), program[i].last_output.end(), &output[program[i].tentative_address]);
    }

    return output;
}