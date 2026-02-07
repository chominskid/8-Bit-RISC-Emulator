#pragma once

#include <stdexcept>

class AssemblerError : public std::runtime_error {
public:
    AssemblerError(const std::string& what);
};