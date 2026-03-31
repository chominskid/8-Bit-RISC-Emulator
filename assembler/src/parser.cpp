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
    return std::isalpha((unsigned char)c) || c == '_' || c == '.' || c == '*';
}

bool is_name_char(char c) {
    return is_first_name_char(c) || std::isdigit(c);
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
        const size_t start = i;
        const size_t end = read_string(is_name_char);
        Origin origin{start, end};
        std::string name = str.substr(start, end - start);
        i = end;

        if (i < str.size() && str[i] == ':') {
            if (KEYWORDS.contains(name))
                throw AssemblerError(origin, "Label name is a reserved keyword.");
            ++i;
            return std::make_unique<LabelDeclaration>(std::move(name), origin);
        } else if (auto it = KEYWORDS.find(name); it != KEYWORDS.end()) {
            return it->second(origin);
        } else {
            return std::make_unique<LabelArg>(std::move(name), origin);
        }
    };

    auto read_string_literal = [&] () -> TokenPtr {
        const size_t start = i;
        std::vector<uint8_t> string;

        auto parse_escape = [&] () {
            switch (str[i]) {
                case '0':
                    string.push_back(0);
                    break;
                case 'n':
                    string.push_back('\n');
                    break;
                case 'r':
                    string.push_back('\r');
                    break;
                case '\\':
                    string.push_back('\\');
                    break;
                case '"':
                    string.push_back('"');
                    break;
                default:
                    throw AssemblerError(Origin{i - 1, i + 1}, "Invalid escape sequence in string literal.");
            }
        };

        for (++i; i < str.size(); ++i) {
            switch (str[i]) {
            case '"': {
                const size_t end = i;
                ++i;
                return std::make_unique<Data>(std::move(string), Origin{start, end});
            } case '\\':
                ++i;
                parse_escape();
                break;
            default:
                string.push_back(str[i]);
                break;
            }
        }

        throw AssemblerError(Origin{start, i}, "String literal not terminated.");
    };

    auto read_number = [&] () -> TokenPtr {
        const size_t start = i;
        const bool negative = str[i] == '-';
        if (negative)
            ++i;

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
        return std::make_unique<IntegerArg>(std::move(num), base, negative, Origin{start, end});
    };

    auto parse_move_directive = [&] () {
        if (current_statement.size() != 2 || current_statement[1]->type() != Token::Type::INTEGER)
            throw AssemblerError(Origin{current_statement.front()->origin.start, current_statement.back()->origin.end}, "Invalid arguments to move directive.");
        const auto address = current_statement[1]->get<IntegerArg>().as<size_t>();
        program.move(address);
    };

    auto parse_byte_directive = [&] () {
        const Origin origin{current_statement.front()->origin.start, current_statement.back()->origin.end};
        std::vector<uint8_t> data;
        for (size_t i = 1; i < current_statement.size(); ++i) {
            switch (current_statement[i]->type()) {
            case Token::Type::INTEGER:
                data.emplace_back(current_statement[i]->get<IntegerArg>().as<uint8_t>());
                break;
            case Token::Type::DATA: {
                const std::vector<uint8_t>& data2 = current_statement[i]->get<Data>().data;
                data.insert(data.end(), data2.begin(), data2.end());
                break;
            }
            default:
                throw AssemblerError(current_statement[i]->origin, "Invalid argument for byte directive.");
            }
        }
        program.add_data(data, origin);
    };

    auto parse_bytes_directive = [&] () {
        const Origin origin{current_statement.front()->origin.start, current_statement.back()->origin.end};
        if (current_statement.size() == 1)
            throw AssemblerError(origin, "Expected size for bytes directive.");
        else if (current_statement.size() > 3)
            throw AssemblerError(origin, "Too many arguments for bytes directive.");

        if (current_statement[1]->type() != Token::Type::INTEGER)
            throw AssemblerError(current_statement[1]->origin, "Invalid size for bytes directive.");

        const size_t size = current_statement[1]->get<IntegerArg>().as<size_t>();
        
        uint8_t fill = 0;
        if (current_statement.size() == 3) {
            if (current_statement[2]->type() != Token::Type::INTEGER)
                throw AssemblerError(current_statement[i]->origin, "Invalid default value argument for bytes directive.");
            fill = current_statement[2]->get<IntegerArg>().as<uint8_t>();
        }

        program.add_data(std::vector<uint8_t>(size, fill), origin);
    };

    auto parse_directive = [&] (Directive::Value d) {
        switch (d) {
        case Directive::Value::MOVE:
            parse_move_directive();
            break;
        case Directive::Value::BYTE:
            parse_byte_directive();
            break;
        case Directive::Value::BYTES:
            parse_bytes_directive();
            break;
        default:
            break;
        }
    };

    auto push_statement = [&] () {
        if (current_statement.size() == 0)
            return;

        switch (current_statement[0]->type()) {
        case Token::Type::OPCODE:
            program.add_instruction(std::move(current_statement));
            break;
        case Token::Type::DIRECTIVE:
            parse_directive(current_statement[0]->get<Directive>().value);
            break;
        default:
            throw AssemblerError(current_statement.front()->origin, "Invalid statement.");
        }

        current_statement.clear();
    };

    bool in_comment = false;
    bool in_comment_block = false;

    for (; i < str.size();) {
        if (is_space(str[i])) {
            if (str[i] == '\n') {
                ++line;
                if (in_comment && !in_comment_block)
                    in_comment = false;
            }
            ++i;
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
                i += 2;
            } else {
                in_comment = true;
                ++i;
            }
            continue;
        }

        if (in_comment) {
            ++i;
            continue;
        }

        TokenPtr token;

        if (is_first_name_char(str[i]))
            token = read_name();
        else if (is_digit(str[i], 10) || (str[i] == '-' && is_digit(next_char(), 10)))
            token = read_number();
        else if (str[i] == '"')
            token = read_string_literal();
        else
            throw AssemblerError(Origin{i, i + 1}, std::format("Unexpected character \'{}\'.", str[i]));

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
            program.add_label(std::move(token->get<LabelDeclaration>().value), token->origin);
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