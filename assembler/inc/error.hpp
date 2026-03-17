#pragma once

#include <stdexcept>

struct Origin {
    size_t start;
    size_t end;
};

class AssemblerError : public std::runtime_error {
public:
    Origin origin;
    
    AssemblerError(Origin origin, const std::string& what);
};