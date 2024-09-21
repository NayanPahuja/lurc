#include "url_parser.h"
#include <regex>
#include <stdexcept>
#include <limits>

/// @brief Validates and converts a port string to a uint16_t value.
/// @param portStr The port string from the URL.
/// @return The validated port number or the default port (80) if the string is empty.
/// @throws runtime_error if the port number is invalid or out of range.
uint16_t UrlParser::validatePort(const std::string& portStr) {
    if (portStr.empty()) {
        return 80;  // Default HTTP port
    }

    try {
        long port = std::stol(portStr);
        if (port < 0 || port > std::numeric_limits<uint16_t>::max()) {
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
    std::regex urlRegex("(http)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)");
    std::smatch match;

    if (std::regex_match(url, match, urlRegex)) {
        result.protocol = match[1];
        result.host = match[2];
        result.port = validatePort(match[3]);
        //Can be made into a ternary operation! TODO!
        if(match[4].length() == 0) {
            result.path = "/";
        }
        else {
            result.path = match[4];
        }
    } else {
        throw std::runtime_error("INVALID URL FORMAT!");
    }

    return result;
}
