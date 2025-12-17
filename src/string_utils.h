#pragma once

#include <string>

// Utility function for string processing
static std::string to_lower(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    for (char c : str) {
        result +=
            static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}