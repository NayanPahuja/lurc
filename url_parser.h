    #pragma once
    #include <string>
    #include <map>


    /// @brief Map used to get default protocl against port.
    /// @return The default port against given protocol.
    const std::map<std::string, uint16_t> protocolToPort {
        {"http",80},
        {"https",443}
    };

    /// @brief Struct representing a parsed URL.
    /// @param protocol The protocol used, expected to be HTTP.
    /// @param host The hostname or domain of the URL.
    /// @param port The port number, defaulting to 80 for HTTP if not specified.
    /// @param path The path component of the URL.
    struct ParsedUrl {
        std::string protocol;
        std::string host;
        uint16_t port;
        std::string path;
    };

    class UrlParser {
    public:
        /// @brief Parses a given URL into its components.
        /// @param url The URL string to parse.
        /// @return A ParsedUrl struct containing the parsed URL components.
        static ParsedUrl parse(const std::string& url);

    private:
        /// @brief Validates the port string and converts it to a port number.
        /// @param portStr The string representation of the port.
        /// @return A valid port number (uint16_t).
        /// @throws runtime_error if the port is out of range or invalid.
        static uint16_t getDefaultPort(const std::string& protocol);
        static uint16_t validatePort(const std::string& protocol,const std::string& portStr);
    };
