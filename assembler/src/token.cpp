#include "../inc/parser.hpp"
#include <format>

Token::~Token() = default;

TokenPtr Token::clone() const {
    return std::make_unique<Token>();
}

Token::Type Token::type() const {
    throw std::runtime_error("bruh");
}



IntegerArg::IntegerArg(std::string&& value, uint8_t base, bool negative) :
    value(std::forward<std::string>(value)),
    base(base),
    negative(negative)
{}

TokenPtr IntegerArg::clone() const {
    return std::make_unique<IntegerArg>(std::string(value), base, negative);
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



Opcode::Opcode(Value value) :
    value(value)
{}

TokenPtr Opcode::clone() const {
    return std::make_unique<Opcode>(value);
}

Token::Type Opcode::type() const {
    return TYPE;
}



Condition::Condition(JumpCond cond, bool negate) :
    cond(cond),
    negate(negate)
{}

TokenPtr Condition::clone() const {
    return std::make_unique<Condition>(cond, negate);
}

Token::Type Condition::type() const {
    return TYPE;
}



DataRegisterArg::DataRegisterArg(Register value) :
    value(value)
{}

TokenPtr DataRegisterArg::clone() const {
    return std::make_unique<DataRegisterArg>(value);
}

Token::Type DataRegisterArg::type() const {
    return TYPE;
}



WideRegisterArg::WideRegisterArg(Value value) :
    value(value)
{}

TokenPtr WideRegisterArg::clone() const {
    return std::make_unique<WideRegisterArg>(value);
}

Token::Type WideRegisterArg::type() const {
    return TYPE;
}



Directive::Directive(Value value) :
    value(value)
{}

TokenPtr Directive::clone() const {
    return std::make_unique<Directive>(value);
}

Token::Type Directive::type() const {
    return TYPE;
}



LabelArg::LabelArg(std::string&& value) :
    value(std::forward<std::string>(value))
{}

TokenPtr LabelArg::clone() const {
    return std::make_unique<LabelArg>(std::string(value));
}

Token::Type LabelArg::type() const {
    return TYPE;
}



LabelDeclaration::LabelDeclaration(std::string&& value) :
    value(std::forward<std::string>(value))
{}

TokenPtr LabelDeclaration::clone() const {
    return std::make_unique<LabelDeclaration>(std::string(value));
}

Token::Type LabelDeclaration::type() const {
    return TYPE;
}