#pragma once

#include "parser.hpp"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

class Signature {
private:
    friend class SignatureHasher;

    std::unique_ptr<Token::Type[]> arg_types;
    size_t num_args;
    Opcode::Value opcode;

public:
    Signature(Opcode::Value opcode, const std::vector<TokenPtr>& args);
    Signature(Opcode::Value opcode, const std::vector<Token::Type>& args = {});
    Signature(const Signature& other);

    bool operator==(const Signature& other) const;
    std::string to_string() const;
};

struct Encoder {
    class Result {
    private:
        static constexpr size_t ERROR = ~(size_t)0;

        size_t _size;
        union {
            std::unique_ptr<std::string> _error;
            std::unique_ptr<uint8_t[]> _data;
            uint8_t _sdata[std::max(sizeof(_error), sizeof(_data))];
        };
    
    public:
        template <typename Int>
        requires std::integral<Int>
        Result(Int data) :
            _size(sizeof(Int))
        {
            if (_size > sizeof(_sdata))
                _data = std::make_unique<uint8_t[]>(_size);
        
            const std::make_unsigned_t<Int> udata = data;

            auto it = begin();
            for (size_t i = 0; i < sizeof(Int); ++i)
                *(it++) = udata >> (8 * (sizeof(Int) - i - 1));
        }

        // template <typename Range>
        // requires std::integral<std::ranges::range_value_t<Range>> && std::ranges::sized_range<Range>
        // Result (const Range& range) :
        //     _size(sizeof(std::ranges::range_value_t<Range>) * std::ranges::size(range))
        // {
        //     using UInt = std::make_unsigned_t<std::ranges::range_value_t<Range>>;

        //     if (_size > sizeof(_sdata))
        //         _data = std::make_unique<uint8_t[]>(_size);

        //     auto it = begin();
        //     for (UInt word: range) {
        //         for (size_t i = 0; i < sizeof(UInt); ++i)
        //             *(it++) = word >> (8 * (sizeof(UInt) - i - 1));
        //     }
        // }



        Result();
        Result(Result&& other);
        Result(const std::string& str);
        ~Result();

        bool has_value() const;
        size_t size() const;
        std::string& error();
        const std::string& error() const;
        
        uint8_t* begin();
        const uint8_t* begin() const;
        const uint8_t* cbegin() const;
        
        uint8_t* end();
        const uint8_t* end() const;
        const uint8_t* cend() const;

        void clear();
        void append(const Result& other);
        Result& operator=(Result&& other);
    };

    using Function = Result(size_t address, const std::vector<TokenPtr>& args);

    const std::optional<size_t> size;
    Function* const encode;
};

struct Instruction {
    const Signature signature;
    const bool independent; // True if this instruction's encoding is the same regardless of its address or the address of any other object.
    const std::vector<Encoder> encoders; // Must be sorted by non-decreasing size (variable size at the end).
};

class SignatureHasher {
private:
    [[no_unique_address]] std::hash<Opcode::Value> opcode_hasher;
    [[no_unique_address]] std::hash<Token::Type> args_hasher;

public:
    using is_transparent = void;

    size_t operator()(const Signature& signature) const;
    size_t operator()(const Instruction& instruction) const;
};

class SignatureEqual {
public:
    using is_transparent = void;

    template <typename A, typename B>
    bool operator()(const A& a, const B& b) const {
        const Signature* _a;
        const Signature* _b;

        if constexpr (std::is_same_v<A, Instruction>)
            _a = &a.signature;
        else
            _a = &a;

        if constexpr (std::is_same_v<B, Instruction>)
            _b = &b.signature;
        else
            _b = &b;

        return *_a == *_b;
    }
};

const extern std::unordered_set<Instruction, SignatureHasher, SignatureEqual> INSTRUCTIONS;