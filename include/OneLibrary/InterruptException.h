#pragma once

#include <iostream>
#include <stdexcept>
#include <exception>
#include <string>

namespace ol
{
class InterruptException : public std::exception
{
public:
    InterruptException() noexcept {}
    [[nodiscard]] const char* what() const noexcept override
    {
        return "Interrupted";
    }
};
}
