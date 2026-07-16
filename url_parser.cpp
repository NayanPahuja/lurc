#include "url_parser.h"
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <cctype>

namespace {
bool hasWhitespace(const std::string& value) {
    return std::any_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c);
    });
}
}


///This can probably be improved by making a enum of existing/supported protocols. and map them to each other than assuming port exists in our map.
///Should add a precheck in protocol only before getting and validating port.

/// @brief Validates and converts a protocol string to a uint16_t value i.e the default port.
/// @param protocol The port string from the URL.
/// @return The default port `[HTTP:80]` or `[HTTPS:443]`against the given string
/// @throws runtime_error if the port number is not a default port of an existing protocol
std::uint16_t UrlParser::getDefaultPort(const std::string & protocol) {
    // Use the `find` method to safely access the port number associated with the protocol.
    auto it = protocolToPort.find(protocol);
    if (it != protocolToPort.end()) {
        return it->second;
    }
    // Return a default port value if the protocol is not found.
    throw std::runtime_error("Unknown protocol: " + protocol);
}

/// @brief Validates and converts a port string to a uint16_t value.
/// @param portStr The port string from the URL.
/// @return The validated port number or the default port (80)/(443) if the string is empty.
/// @throws runtime_error if the port number is invalid or out of range.
uint16_t UrlParser::validatePort(const std::string &protocol, const std::string& portStr) {
    if (portStr.empty()) {
        return getDefaultPort(protocol);  // Default port of that protocl
    }

    if (!std::all_of(portStr.begin(), portStr.end(), [](unsigned char c) { return std::isdigit(c); })) {
        throw std::runtime_error("Invalid port number: " + portStr);
    }

    try {
        long port = std::stol(portStr);
        if (port <= 0 || port > std::numeric_limits<uint16_t>::max()) {
            throw std::out_of_range("Port number out of range");
        }
        return static_cast<uint16_t>(port);
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid port number: " + portStr);
    }
}

/// @brief Parses the given URL and extracts its components.
/// @param url The URL string to parse.
/// @return A ParsedUrl struct containing the protocol, host, port, and path.
/// @throws runtime_error if the URL format is invalid.
ParsedUrl UrlParser::parse(const std::string& url) {
    ParsedUrl result;
    if (url.empty() || hasWhitespace(url)) {
        throw std::runtime_error("INVALID URL FORMAT!");
    }

    const std::size_t schemeSeparatorPos = url.find("://");
    if (schemeSeparatorPos == std::string::npos || schemeSeparatorPos == 0) {
        throw std::runtime_error("INVALID URL FORMAT!");
    }

    result.protocol = url.substr(0, schemeSeparatorPos);
    std::transform(result.protocol.begin(), result.protocol.end(), result.protocol.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (result.protocol != "http" && result.protocol != "https") {
        throw std::runtime_error("Unknown protocol: " + result.protocol);
    }

    const std::size_t authorityStartPos = schemeSeparatorPos + 3;
    const std::size_t authorityEndPos = url.find_first_of("/?#", authorityStartPos);
    const std::string authority = url.substr(authorityStartPos, authorityEndPos - authorityStartPos);
    if (authority.empty() || authority.find('@') != std::string::npos) {
        throw std::runtime_error("INVALID URL FORMAT!");
    }

    std::string portStr;
    if (authority.front() == '[') {
        const std::size_t hostEndPos = authority.find(']');
        if (hostEndPos == std::string::npos) {
            throw std::runtime_error("INVALID URL FORMAT!");
        }
        result.host = authority.substr(0, hostEndPos + 1);
        if (hostEndPos + 1 < authority.size()) {
            if (authority[hostEndPos + 1] != ':') {
                throw std::runtime_error("INVALID URL FORMAT!");
            }
            portStr = authority.substr(hostEndPos + 2);
        }
    } else {
        const std::size_t colonPos = authority.rfind(':');
        if (colonPos != std::string::npos) {
            result.host = authority.substr(0, colonPos);
            portStr = authority.substr(colonPos + 1);
        } else {
            result.host = authority;
        }
    }

    if (result.host.empty() || hasWhitespace(result.host)) {
        throw std::runtime_error("INVALID URL FORMAT!");
    }
    result.port = validatePort(result.protocol, portStr);

    if (authorityEndPos == std::string::npos || url[authorityEndPos] == '#') {
        result.path = "/";
    } else {
        const std::size_t fragmentPos = url.find('#', authorityEndPos);
        result.path = url.substr(authorityEndPos, fragmentPos - authorityEndPos);
        if (!result.path.empty() && result.path.front() == '?') {
            result.path.insert(result.path.begin(), '/');
        }
    }

    if (result.path.empty()) {
        result.path = "/";
    }

    return result;
}
