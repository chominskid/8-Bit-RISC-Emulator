#include "../inc/parser.hpp"
#include "../inc/error.hpp"
#include "../inc/program.hpp"
#include <algorithm>
#include <cctype>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

Token::~Token() = default;

Token::Type Token::type() const {
    throw std::runtime_error("bruh");
}

IntegerArg::IntegerArg(std::string&& value, uint8_t base, bool negative) :
    value(std::forward<std::string>(value)),
    base(base),
    negative(negative)
{}

Token::Type IntegerArg::type() const {
    return TYPE;
}

std::string IntegerArg::to_string() const {
    const char* base_str = "";
    switch (base) {
        case 2: base_str = "0b"; break;
        case 8: base_str = "0"; break;
        case 16: base_str = "0x"; break;
        default: break;
    }
    return std::format("{}{}{}", negative ? "-" : "", base_str, value);
}

Opcode::Opcode(Value value) :
    value(value)
{}

Token::Type Opcode::type() const {
    return TYPE;
}

Condition::Condition(JumpCond cond, bool negate) :
    negate(negate),
    cond(cond)
{}

Token::Type Condition::type() const {
    return TYPE;
}

DataRegisterArg::DataRegisterArg(Register value) :
    value(value)
{}

Token::Type DataRegisterArg::type() const {
    return TYPE;
}

WideRegisterArg::WideRegisterArg(Value value) :
    value(value)
{}

const std::unordered_map<std::string, WideRegisterArg::Value> WideRegisterArg::TRANSL {

};

Token::Type WideRegisterArg::type() const {
    return TYPE;
}

Directive::Directive(Value value) :
    value(value)
{}

Token::Type Directive::type() const {
    return TYPE;
}

LabelArg::LabelArg(std::string&& value) :
    value(std::forward<std::string>(value))
{}

Token::Type LabelArg::type() const {
    return TYPE;
}

LabelDeclaration::LabelDeclaration(std::string&& value) :
    value(std::forward<std::string>(value))
{}

Token::Type LabelDeclaration::type() const {
    return TYPE;
}

std::string to_lower(const std::string& str) {
    std::string str2;
    str2.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(str2), [] (char c) {
        return std::tolower((unsigned char)c); 
    });
    return str2;
}

bool is_space(char c) {
    return std::isspace((unsigned char)c);
}

bool is_first_name_char(char c) {
    return std::isalpha((unsigned char)c) || c == '_';
}

bool is_name_char(char c) {
    return is_first_name_char(c) || std::isdigit(c) || c == '.';
}

bool is_digit(char c, uint8_t base) {
    return IntegerArg::_digit_value(c, base) < base;
}

const std::unordered_map<std::string, TokenPtr(*)(const std::string&)> KEYWORDS {
    { "nop", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::NOP); }},
    { "add", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::ADD); }},
    { "adc", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::ADC); }},
    { "sub", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::SUB); }},
    { "sbc", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::SBC); }},
    { "cmp", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::CMP); }},
    { "cmc", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::CMC); }},
    { "and", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::AND); }},
    { "or", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::OR); }},
    { "xor", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::XOR); }},
    { "shl", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::SHL); }},
    { "shr", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::SHR); }},
    { "mov", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::MOV); }},
    { "mvh", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::MVH); }},
    { "tsb", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::TSB); }},
    { "seb", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::SEB); }},
    { "jmp", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::JMP); }},
    { "rjmp", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::RJMP); }},
    { "jbl", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::JBL); }},
    { "jbh", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::JBH); }},
    { "call", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::CALL); }},
    { "rcall", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::RCALL); }},
    { "cbl", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::CBL); }},
    { "cbh", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::CBH); }},
    { "ret", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::RET); }},
    { "retcall", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::RETCALL); }},
    { "ld", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::LD); }},
    { "ldr", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::LDR); }},
    { "lds", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::LDS); }},
    { "ldf", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::LDF); }},
    { "st", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::ST); }},
    { "sts", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::STS); }},
    { "stf", [] (const std::string& str) -> TokenPtr { return std::make_unique<Opcode>(Opcode::Value::STF); }},

    { "c", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::C, false); }},
    { "gteu", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::C, false); }},
    { "v", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::V, false); }},
    { "n", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::N, false); }},
    { "z", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::Z, false); }},
    { "eq", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::Z, false); }},
    { "gt", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::G, false); }},
    { "gte", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::GE, false); }},
    { "gtu", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::GU, false); }},
    { "nc", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::C, true); }},
    { "ltu", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::C, true); }},
    { "nv", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::V, true); }},
    { "nn", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::N, true); }},
    { "nz", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::Z, true); }},
    { "ne", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::Z, true); }},
    { "lte", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::G, true); }},
    { "lt", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::GE, true); }},
    { "lteu", [] (const std::string& str) -> TokenPtr { return std::make_unique<Condition>(JumpCond::GU, true); }},

    { "ra.l", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::RA_L); }},
    { "ra.h", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::RA_H); }},
    { "sr", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::SR); }},
    { "sp", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::SP); }},
    { "ga", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GA); }},
    { "fp", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GA); }},
    { "gb", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GB); }},
    { "gc", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GC); }},
    { "gd", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GD); }},
    { "ge.l", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GE_L); }},
    { "ge.h", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GE_H); }},
    { "gf.l", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GF_L); }},
    { "gf.h", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GF_H); }},
    { "gg.l", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GG_L); }},
    { "gg.h", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GG_H); }},
    { "gh.l", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GH_L); }},
    { "gh.h", [] (const std::string& str) -> TokenPtr { return std::make_unique<DataRegisterArg>(Register::GH_H); }},

    { "ge", [] (const std::string& str) -> TokenPtr { return std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GE); }},
    { "gf", [] (const std::string& str) -> TokenPtr { return std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GF); }},
    { "gg", [] (const std::string& str) -> TokenPtr { return std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GG); }},
    { "gh", [] (const std::string& str) -> TokenPtr { return std::make_unique<WideRegisterArg>(WideRegisterArg::Value::GH); }},
};

Program parse(const std::string& str) {
    Program program;

    size_t i = 0;
    size_t line = 0;
    std::vector<std::unique_ptr<Token>> current_statement;

    auto next_char = [&] () {
        if (i + 1 >= str.size())
            return '\0';
        return str[i + 1];
    };

    auto read_string = [&] (const std::function<bool(char)>& pred) -> size_t {
        size_t j = i;
        for (; j < str.size(); ++j) {
            if (!pred(str[j]))
                break;
        }
        return j;
    };

    auto read_name = [&] () -> TokenPtr {
        const size_t end = read_string(is_name_char);
        std::string name = str.substr(i, end - i);
        i = end;
        

        if (i < str.size() && str[i] == ':') {
            if (KEYWORDS.contains(name))
                throw AssemblerError("Label name is a reserved keyword.");
            ++i;
            return std::make_unique<LabelDeclaration>(std::move(name));
        } else if (auto it = KEYWORDS.find(name); it != KEYWORDS.end()) {
            return it->second(name);
        } else {
            return std::make_unique<LabelArg>(std::move(name));
        }
    };

    auto read_number = [&] () -> TokenPtr {
        const bool negative = str[i] == '-';

        uint8_t base = 10;
        if (str[i] == '0') {
            switch (next_char()) {
            case 'x':
                base = 16;
                i += 2;
                break;
            case 'b':
                base = 2; 
                i += 2;
                break;
            default:
                if (is_digit(next_char(), 8)) {
                    base = 8;
                    i += 1;
                }
                break;
            }
        }

        const size_t end = read_string(std::bind(is_digit, std::placeholders::_1, base));
        std::string num = str.substr(i, end - i);
        i = end;
        return std::make_unique<IntegerArg>(std::move(num), base, negative);
    };

    auto push_statement = [&] () {
        program.add_instruction(std::move(current_statement));
        current_statement.clear();
    };

    bool in_comment = false;
    bool in_comment_block = false;

    for (; i < str.size(); ++i) {
        if (is_space(str[i])) {
            if (str[i] == '\n') {
                ++line;
                if (in_comment && !in_comment_block)
                    in_comment = false;
            }
            continue;
        } else if (str[i] == '#') {
            if (next_char() == '#') {
                if (in_comment_block) {
                    in_comment = false;
                    in_comment_block = false;
                } else if (!in_comment) {
                    in_comment = true;
                    in_comment_block = true;
                }
                i += 1;
            } else
                in_comment = true;
            continue;
        }

        if (in_comment)
            continue;

        TokenPtr token;

        if (is_first_name_char(str[i]))
            token = read_name();
        else if (is_digit(str[i], 10) || (str[i] == '-' && is_digit(next_char(), 10)))
            token = read_number();
        else
            throw AssemblerError(std::format("Unexpected character \'{}\'.", str[i]));

        if (current_statement.size() != 0) {
            switch (token->type()) {
            case Token::Type::OPCODE:
            case Token::Type::DIRECTIVE:
            case Token::Type::LABEL_DECL:
                push_statement();
            default:
                break;
            }
        }

        switch (token->type()) {
        case Token::Type::LABEL_DECL:
            program.add_label(std::move(token->get<LabelDeclaration>().value));
            break;
        case Token::Type::DIRECTIVE:
            break;
        default:
            current_statement.emplace_back(token.release());
            break;
        }
    }

    if (current_statement.size() != 0)
        push_statement();

    return program;
}