#include "../../inc/utils/arg_parse.hpp"

ArgParse::ArgParse(const int argc, const char* argv[]) {
    const char *option_name = NULL;
    bool next_is_option_value = false;

    error = {};

    for (int i = 1; i < argc; i++) {
        auto arg = std::string(argv[i]);
        if (next_is_option_value) {
            options.insert({ std::string(option_name), std::string(arg) });
            next_is_option_value = false;
        } else if (arg.starts_with("--")) {
            option_name = argv[i];
            next_is_option_value = true;
        } else {
            normal_args.push(std::string(argv[i]));
        }
    }

    if (next_is_option_value) {
        error = std::string("Tailing option is missing a value");
    }
}

std::optional<std::string> ArgParse::take_normal() {
    if (normal_args.empty()) {
        return {};
    }

    auto val = normal_args.front();
    normal_args.pop();
    return val;
}

std::optional<std::string> ArgParse::take_option(const char *str) {
    auto key = std::string(str);
    auto it = options.find(key);
    if (it != options.end()) {
        options.erase(key);
        return it->second;
    } else {
        return {};
    }
}

std::optional<std::string> ArgParse::get_error() {
    return error;
}

bool ArgParse::has_remaining() {
    return !(options.empty() && normal_args.empty());
}