#include "../inc/error.hpp"

AssemblerError::AssemblerError(Origin origin, const std::string& what) :
    std::runtime_error(what),
    origin(origin)
{}