#pragma once
#include "url_parser.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>

/// @brief Enumeration of supported HTTP methods.
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

const std::map<std::string, HttpMethod> StringToHttpMethod = {
    {"GET", HttpMethod::GET},
    {"POST", HttpMethod::POST},
    {"DELETE", HttpMethod::DELETE},
    {"PUT", HttpMethod::PUT}
};

const std::map<HttpMethod, std::string> HttpMethodToString = {
    {HttpMethod::GET, "GET"},
    {HttpMethod::POST, "POST"},
    {HttpMethod::PUT, "PUT"},
    {HttpMethod::DELETE, "DELETE"}
};


struct HttpRequest {
    HttpMethod method;
    ParsedUrl url;
    std::map<std::string, std::string> headers;
    std::string data;
    std::string outputFile;
};


/// @brief Struct representing an HTTP response.
/// @param statusLine The status line of the HTTP response.
/// @param headers The headers of the HTTP response.
/// @param body The body of the HTTP response.
struct HttpResponse {
    std::string statusLine;
    std::vector<std::string> headers;
    std::string body;
};

class HttpClient {
public:

    HttpClient();
    ~HttpClient();
    /// @brief Generates an HTTP request string based on the parsed URL and HTTP method.
    /// @param parsedUrl The parsed URL struct.
    /// @param method The HTTP method (GET, POST, etc.).
    /// @return The generated HTTP request string.
    static std::string generateRequest(const HttpRequest& request);

    /// @brief Sends the HTTP request to the server and receives the response.
/// @param request Accepts a request of struct HttpRequest containing relevant info.s
/// @param verbose Flag to enable verbose output of the request and response.
/// @return The HTTP response received from the server.
/// @throws runtime_error if any error occurs during the request.
    HttpResponse sendRequest(const HttpRequest& request, bool verbose);
    void downloadFile(const HttpRequest& request, bool verbose);


private:
    SSL_CTX* ctx;
    void initSSL();
    void cleanSSL();
    SSL* createSSLConnection(int socket, const std::string & hostname);
    bool verifySSLCert(SSL* ssl, const std::string& hostname);

    //helper
    static int createSocket(const std::string& hostname, uint16_t port);
    static void handleSSLError(SSL* ssl, int result);
};
