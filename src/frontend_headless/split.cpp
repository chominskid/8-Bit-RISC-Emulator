#include "../../inc/frontend_headless/split.hpp"
#include <cctype>

bool is_space(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> result;
    size_t last = 0;
    for (size_t i = 0;; ++i) {
        if (is_space(str[i]) || i == str.size()) {
            if (i - last != 0)
                result.emplace_back(str.substr(last, i - last));
            last = i + 1;
            if (i == str.size())
                break;
        }
    }
    result.shrink_to_fit();
    return result;
}