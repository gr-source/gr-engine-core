#include "gr-base/asset/URI.hpp"

#include <stdexcept>

std::string URI::toString() const
{
    std::string result;
    switch (scheme)
    {
        case Scheme::Engine:  result += "engine"; break;
        case Scheme::Project: result += "project"; break;
        case Scheme::Mod:     result += "mod"; break;
        default:
            throw std::runtime_error("Unknown scheme enum");
    }

    return result + "://" + path;
}

URI URI::parse(const std::string& uri)
{
    size_t start = uri.find("://");
    if (start == std::string::npos)
        throw std::runtime_error("Invalid URI: missing scheme");

    URI result;

    std::string schemeStr = uri.substr(0, start);
    result.path = uri.substr(start + 3);

    if (schemeStr == "engine") result.scheme = Scheme::Engine;
    else if (schemeStr == "project") result.scheme = Scheme::Project;
    else if (schemeStr == "mod") result.scheme = Scheme::Mod;
    else
        throw std::runtime_error("Unknown scheme: " + schemeStr);

    return result;
}

