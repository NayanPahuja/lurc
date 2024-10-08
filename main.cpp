#include <iostream>
#include "url_parser.h"
#include "http_client.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

/// @brief Converts a given HTTP method string to an HttpMethod enum.
/// @param methodStr The string representation of the HTTP method.
/// @return The corresponding HttpMethod enum value.
/// @throws runtime_error if the method string is invalid.
HttpMethod parseMethod(const std::string& methodStr) {
    std::string upperMethod = methodStr;
    std::transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);

    auto it = StringToHttpMethod.find(upperMethod);
    if (it != StringToHttpMethod.end()) {
        return it->second;
    }
    throw std::runtime_error("Invalid HTTP method: " + methodStr);
}

/// @brief Main function to handle command-line input and perform HTTP requests.
/// @param argc The count of command-line arguments.
/// @param argv The command-line arguments array.
/// @return 0 on success, 1 on failure.
int main(int argc, char *argv[]) {
    // Check for help option
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        std::cout << "Usage: " << argv[0] << " [-v] [-X <method>] [-H <header>] [-d <data>] [-o <output_file>] [-L] <URL>" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -v                : Verbose output (shows request and response details)" << std::endl;
        std::cout << "  -X <method>       : Specify HTTP method to use (GET, POST, PUT, DELETE)" << std::endl;
        std::cout << "  -H <header>       : Specify a custom header (format: 'Key: Value')" << std::endl;
        std::cout << "  -d <data>         : Send data in the request body (for POST/PUT requests)" << std::endl;
        std::cout << "  -o <output_file>  : Write response to the specified output file" << std::endl;
        std::cout << "  -L                : Follow redirects" << std::endl;
        std::cout << "  <URL>             : The URL to send the request to" << std::endl;
        return 0;
    }

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [-v] [-X <method>] [-H <header>] [-d <data>] [-o <output_file>] [-L] <URL>" << std::endl;
        return 1;
    }

    bool verbose = false;
    bool followsRedirects = false;
    HttpRequest request;
    request.method = HttpMethod::GET;  // Default method

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-L") == 0) {
            followsRedirects = true;
        } else if (strcmp(argv[i], "-X") == 0) {
            if (i + 1 < argc) {
                try {
                    request.method = parseMethod(argv[++i]);
                } catch (const std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: -X option requires a method argument." << std::endl;
                return 1;
            }
        } else if (strcmp(argv[i], "-H") == 0) {
            if (i + 1 < argc) {
                std::string header = argv[++i];
                size_t colonPos = header.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = header.substr(0, colonPos);
                    std::string value = header.substr(colonPos + 1);
                    request.headers[key] = value;
                } else {
                    std::cerr << "Error: Invalid header format. Use 'Key: Value'." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: -H option requires a header argument." << std::endl;
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                request.data = argv[++i];
            } else {
                std::cerr << "Error: -d option requires a data argument." << std::endl;
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if(i + 1 < argc) {
                request.outputFile = argv[++i];
            } else {
                std::cerr << "Error: -o option requires an output file argument." << std::endl;
                return 1;
            }
        } else {
            request.url = UrlParser::parse(argv[i]);
        }
    }

    if (request.url.host.empty()) {
        std::cerr << "Error: URL is required." << std::endl;
        return 1;
    }

    // Add default headers if not provided
    if (request.headers.find("Accept") == request.headers.end()) {
        request.headers["Accept"] = "*/*";
    }
    if (request.headers.find("Connection") == request.headers.end()) {
        request.headers["Connection"] = "close";
    }

    try {
        HttpClient client;
        if(!request.outputFile.empty()) {
            client.downloadFile(request, verbose);
        } else {
            HttpResponse response = client.sendRequest(request, verbose);
            std::cout << response.body << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
