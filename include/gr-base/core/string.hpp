#pragma once

#include <string>

class String {
public:
    static std::string RemoveNumber(const std::string &str);

    static std::string replace_all(std::string text, const std::string &lhs, const std::string &rhs);
    
    template <typename ...Args>
    static std::string format(const std::string &fmt, Args &&...args);
    
};

template <typename... Args>
inline std::string String::format(const std::string &fmt, Args &&...args) {
    auto retVal = snprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...);
    if (retVal <= 0) { return ""; }

    std::string result(retVal, '\0');
    snprintf(result.data(), result.size() + 1, fmt.c_str(), std::forward<Args>(args)...);

    return result;
}
