#include "url_parser.h"
#include <regex>
#include <stdexcept>
#include <limits>


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
    //change the regex for https
    std::regex urlRegex("(https?)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)");
    std::smatch match;

    if (std::regex_match(url, match, urlRegex)) {
        result.protocol = match[1];
        result.host = match[2];
        result.port = validatePort(result.protocol,match[3]);
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
