#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "../../common/inc/encoding.hpp"

class Token {
public:
    enum class Type : uint8_t {
        INTEGER,
        OPCODE,
        CONDITION,
        DATA_REGISTER,
        WIDE_REGISTER,
        DIRECTIVE,
        LABEL,
        LABEL_DECL,
    };

    size_t line;

    virtual ~Token();
    virtual Type type() const;
    
    template <typename T>
    T& get() {
        return *static_cast<T*>(this);
    }

    template <typename T>
    const T& get() const {
        return *static_cast<T*>(this);
    }
};

using TokenPtr = std::unique_ptr<Token>;

class IntegerArg : public Token {
private:
    friend bool is_digit(char c, uint8_t base);
    static inline constexpr uint8_t _digit_value(char c, uint8_t base) {
        uint8_t result = base;
        if (c >= '0' && c <= '9')
            result = c - '0';
        else if (c >= 'a' && c <= 'f')
            result = 10 + c - 'a';
        else if (c >= 'A' && c <= 'F')
            result = 10 + c - 'A';
        return result;
    }

public:
    static constexpr Type TYPE = Type::INTEGER;

    std::string value;
    uint8_t base;
    bool negative;

    IntegerArg(std::string&& value, uint8_t base, bool negative);

    template <typename Int>
    std::optional<Int> try_as() const {
        using UInt = std::make_unsigned_t<Int>;
        static constexpr UInt MSB = (UInt)1 << (std::numeric_limits<UInt>::digits - 1);

        UInt result = 0;
        for (size_t i = 0; i < value.size(); ++i) {
            // const char c = value[value.size() - i - 1];
            const char c = value[i];
            const UInt result_old = result;
            result *= base;
            if (result / base != result_old)
                return std::nullopt;

            const UInt digit_value = _digit_value(c, base);
            result += digit_value;
            if (result < digit_value)
                return std::nullopt;
        }

        if (negative) {
            if (result > MSB)
                return std::nullopt;
            result = ~result + 1;
        } else if (result >= MSB && std::is_signed_v<Int>)
            return std::nullopt;

        return result;
    }

    Type type() const override;
    std::string to_string() const;
};

class Opcode : public Token {
public:
    enum class Value {
        NOP,

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
        MOV,
        MVH,
        TSB,
        SEB,

        JMP,
        RJMP,
        JBL,
        JBH,
        CALL,
        RCALL,
        CBL,
        CBH,
        RET,
        RETCALL,

        LD,
        LDR,
        LDS,
        LDF,

        ST,
        STS,
        STF
    };

    static const std::unordered_map<std::string, Value> TRANSL;
    static constexpr Type TYPE = Type::OPCODE;

    Value value;

    Opcode(Value value);

    Type type() const override;
};

class Condition : public Token {
public:
    JumpCond cond;
    bool negate;

    static constexpr Type TYPE = Type::CONDITION;

    Condition(JumpCond cond, bool negate);

    Type type() const override;
};

class DataRegisterArg : public Token {
public:
    static const std::unordered_map<std::string, Register> TRANSL;
    static constexpr Type TYPE = Type::DATA_REGISTER;

    Register value;

    DataRegisterArg(Register value);
    Type type() const override;
};

class WideRegisterArg : public Token {
public:
    enum class Value : uint8_t {
        GE,
        GF,
        GG,
        GH,
    };

    static const std::unordered_map<std::string, Value> TRANSL;
    static constexpr Type TYPE = Type::WIDE_REGISTER;

    Value value;

    WideRegisterArg(Value value);
    Type type() const override;
};

class Directive : public Token {
public:
    enum class Value {
        MOVE,
        HERE,
    };

    static constexpr Type TYPE = Type::DIRECTIVE;

    Value value;

    Directive(Value value);
    Type type() const override;
};

class LabelArg : public Token {
public:
    static constexpr Type TYPE = Type::LABEL;

    std::string value;
    size_t address;

    LabelArg(std::string&& value);
    Type type() const override;
};

class LabelDeclaration : public Token {
public:
    static constexpr Type TYPE = Type::LABEL_DECL;

    std::string value;

    LabelDeclaration(std::string&& value);
    Type type() const override;
};

class Program parse(const std::string& str);