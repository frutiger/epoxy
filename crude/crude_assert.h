// crude_assert.h
#pragma once

#include <iostream>

#define CRUDE_ASSERT(expr)                                                    \
    if (!(expr)) {                                                            \
        std::cerr << __FILE__ << ':' << __LINE__ << ": "                      \
                  << "Assertion failed: " << #expr << '\n';                   \
        std::abort();                                                         \
    }

