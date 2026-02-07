#include "../inc/error.hpp"

AssemblerError::AssemblerError(const std::string& what) :
    std::runtime_error(what)
{}