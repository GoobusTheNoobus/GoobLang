#pragma once
#include "ansi.h"
#include <iostream>
#include <string>

namespace diagnostic {

inline bool has_error = false;

inline void ReportError(const std::string &str) {
    std::cout << ansi::Bold << ansi::Red << "Error: " << ansi::Reset << str << "\n";
    has_error = true;
}

} // namespace diagnostic