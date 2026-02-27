#include "../inc/parser.hpp"
#include "../inc/error.hpp"
#include "../inc/program.hpp"
#include <algorithm>
#include <cctype>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <vector>

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
            return it->second->clone();
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