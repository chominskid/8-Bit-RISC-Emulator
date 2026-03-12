#pragma once

#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>

template <typename T>
class Translator {
private:
    std::unordered_map<std::string, T> _to;
    std::unordered_map<T, std::string> _from;

public:
    Translator(std::initializer_list<std::pair<T, std::string>> map) {
        _to.reserve(map.size());
        _from.reserve(map.size());

        for (const auto&[x, str]: map) {
            _to.emplace(str, x);
            _from.emplace(x, str);
        }
    }

    std::optional<T> to(const std::string& str) const {
        auto it = _to.find(str);
        if (it != _to.end())
            return *it;
        else
            return std::nullopt;
    }

    std::optional<std::string> from(const T& x) const {
        auto it = _from.find(x);
        if (it != _from.end())
            return *it;
        else
            return std::nullopt;
    }
};