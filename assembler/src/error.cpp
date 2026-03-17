#include "../inc/error.hpp"

AssemblerError::AssemblerError(Origin origin, const std::string& what) :
    origin(origin),
    std::runtime_error(what)
{}