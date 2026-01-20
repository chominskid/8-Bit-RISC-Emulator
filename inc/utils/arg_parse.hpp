#pragma once
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>

class ArgParse {
private:
    std::queue<std::string> normal_args;
    std::unordered_map<std::string, std::string> options;
    std::optional<std::string> error;

public:
    ArgParse() = delete;
    ArgParse(const ArgParse&) = delete;
    ArgParse(ArgParse&&) = delete;

    ArgParse(const int argc, const char* argv[]);

    /// Takes one normal argument
    std::optional<std::string> take_normal();

    /// Takes one flag (and removes it)
    /// A second call to this function with the same arguments
    /// will always return false.
    std::optional<std::string> take_option(const char *str);

    /// Returns an error if there was an error when parsing arguments.
    std::optional<std::string> get_error();

    /// Checks if there are remaining flags or arguments that have not been
    /// taken.
    bool has_remaining();
};
