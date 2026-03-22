#include "../inc/parser.hpp"
#include <format>

Token::Token(Origin origin) :
    origin(origin)
{}

Token::~Token() = default;

TokenPtr Token::clone() const {
    return std::make_unique<Token>(origin);
}

Token::Type Token::type() const {
    throw std::runtime_error("bruh");
}



Data::Data(Origin origin) :
    Token(origin),
    data()
{}

Data::Data(std::vector<uint8_t>&& data, Origin origin) :
    Token(origin),
    data(std::forward<std::vector<uint8_t>>(data))
{}

Data::Data(const std::vector<uint8_t>& data, Origin origin) :
    Token(origin),
    data(data)
{}

TokenPtr Data::clone() const {
    return std::make_unique<Data>(data, origin);
}

Token::Type Data::type() const {
    return TYPE;
}

char hex_char(char x) {
    if (x <= 9)
        return '0' + x;
    else
        return 'A' + x - 10;
}

std::string Data::to_string() const {
    std::string str("[");
    for (size_t i = 0; i < str.size(); ++i) {
        str.push_back(hex_char(str[i] >> 4));
        str.push_back(hex_char(str[i] & 0xF));
        if (i + 1 < str.size())
            str.append(", ");
    }
    str.push_back(']');
    return str;
}



IntegerArg::IntegerArg(std::string&& value, uint8_t base, bool negative, Origin origin) :
    Token(origin),
    value(std::forward<std::string>(value)),
    base(base),
    negative(negative)
{}

TokenPtr IntegerArg::clone() const {
    return std::make_unique<IntegerArg>(std::string(value), base, negative, origin);
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



Opcode::Opcode(Value value, Origin origin) :
    Token(origin),
    value(value)
{}

TokenPtr Opcode::clone() const {
    return std::make_unique<Opcode>(value, origin);
}

Token::Type Opcode::type() const {
    return TYPE;
}



Condition::Condition(JumpCond cond, bool negate, Origin origin) :
    Token(origin),
    cond(cond),
    negate(negate)
{}

TokenPtr Condition::clone() const {
    return std::make_unique<Condition>(cond, negate, origin);
}

Token::Type Condition::type() const {
    return TYPE;
}



DataRegisterArg::DataRegisterArg(Register value, Origin origin) :
    Token(origin),
    value(value)
{}

TokenPtr DataRegisterArg::clone() const {
    return std::make_unique<DataRegisterArg>(value, origin);
}

Token::Type DataRegisterArg::type() const {
    return TYPE;
}



WideRegisterArg::WideRegisterArg(Value value, Origin origin) :
    Token(origin),
    value(value)
{}

TokenPtr WideRegisterArg::clone() const {
    return std::make_unique<WideRegisterArg>(value, origin);
}

Token::Type WideRegisterArg::type() const {
    return TYPE;
}



Directive::Directive(Value value, Origin origin) :
    Token(origin),
    value(value)
{}

TokenPtr Directive::clone() const {
    return std::make_unique<Directive>(value, origin);
}

Token::Type Directive::type() const {
    return TYPE;
}



LabelArg::LabelArg(std::string&& value, Origin origin) :
    Token(origin),
    value(std::forward<std::string>(value))
{}

TokenPtr LabelArg::clone() const {
    return std::make_unique<LabelArg>(std::string(value), origin);
}

Token::Type LabelArg::type() const {
    return TYPE;
}



LabelDeclaration::LabelDeclaration(std::string&& value, Origin origin) :
    Token(origin),
    value(std::forward<std::string>(value))
{}

TokenPtr LabelDeclaration::clone() const {
    return std::make_unique<LabelDeclaration>(std::string(value), origin);
}

Token::Type LabelDeclaration::type() const {
    return TYPE;
}