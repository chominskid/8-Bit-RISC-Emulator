#include "../inc/parser.hpp"
#include <format>

Token::Token(size_t line) : line(line)
{}

Token::~Token() = default;

TokenPtr Token::clone() const {
    return std::make_unique<Token>(line);
}

Token::Type Token::type() const {
    throw std::runtime_error("bruh");
}



IntegerArg::IntegerArg(size_t line, std::string&& value, uint8_t base, bool negative) :
    value(std::forward<std::string>(value)),
    base(base),
    negative(negative),
    Token(line)
{}

TokenPtr IntegerArg::clone() const {
    return std::make_unique<IntegerArg>(line, std::string(value), base, negative);
}

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



Opcode::Opcode(size_t line, Value value) :
    value(value),
    Token(line)
{}

TokenPtr Opcode::clone() const {
    return std::make_unique<Opcode>(line, value);
}

Token::Type Opcode::type() const {
    return TYPE;
}



Condition::Condition(size_t line, JumpCond cond, bool negate) :
    cond(cond),
    negate(negate),
    Token(line)
{}

TokenPtr Condition::clone() const {
    return std::make_unique<Condition>(line, cond, negate);
}

Token::Type Condition::type() const {
    return TYPE;
}



DataRegisterArg::DataRegisterArg(size_t line, Register value) :
    value(value),
    Token(line)
{}

TokenPtr DataRegisterArg::clone() const {
    return std::make_unique<DataRegisterArg>(line, value);
}

Token::Type DataRegisterArg::type() const {
    return TYPE;
}



WideRegisterArg::WideRegisterArg(size_t line, Value value) :
    value(value),
    Token(line)
{}

TokenPtr WideRegisterArg::clone() const {
    return std::make_unique<WideRegisterArg>(line, value);
}

Token::Type WideRegisterArg::type() const {
    return TYPE;
}



Directive::Directive(size_t line, Value value) :
    value(value),
    Token(line)
{}

TokenPtr Directive::clone() const {
    return std::make_unique<Directive>(line, value);
}

Token::Type Directive::type() const {
    return TYPE;
}



LabelArg::LabelArg(size_t line, std::string&& value) :
    value(std::forward<std::string>(value)),
    Token(line)
{}

TokenPtr LabelArg::clone() const {
    return std::make_unique<LabelArg>(line, std::string(value));
}

Token::Type LabelArg::type() const {
    return TYPE;
}



LabelDeclaration::LabelDeclaration(size_t line, std::string&& value) :
    value(std::forward<std::string>(value)),
    Token(line)
{}

TokenPtr LabelDeclaration::clone() const {
    return std::make_unique<LabelDeclaration>(line, std::string(value));
}

Token::Type LabelDeclaration::type() const {
    return TYPE;
}