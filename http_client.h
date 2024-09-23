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
    GET,     ///< HTTP GET method for retrieving resources.
    POST,    ///< HTTP POST method for sending data to the server.
    PUT,     ///< HTTP PUT method for updating resources on the server.
    DELETE   ///< HTTP DELETE method for removing resources from the server.
};

/// @brief Map to convert the input string and validate against our existing supported methods.
const std::map<std::string, HttpMethod> StringToHttpMethod = {
    {"GET", HttpMethod::GET},       ///< Maps "GET" string to HttpMethod::GET.
    {"POST", HttpMethod::POST},     ///< Maps "POST" string to HttpMethod::POST.
    {"DELETE", HttpMethod::DELETE}, ///< Maps "DELETE" string to HttpMethod::DELETE.
    {"PUT", HttpMethod::PUT}        ///< Maps "PUT" string to HttpMethod::PUT.
};

/// @brief Map to convert the output METHOD and use that to generate request.
const std::map<HttpMethod, std::string> HttpMethodToString = {
    {HttpMethod::GET, "GET"},       ///< Maps HttpMethod::GET to "GET" string.
    {HttpMethod::POST, "POST"},     ///< Maps HttpMethod::POST to "POST" string.
    {HttpMethod::PUT, "PUT"},       ///< Maps HttpMethod::PUT to "PUT" string.
    {HttpMethod::DELETE, "DELETE"}  ///< Maps HttpMethod::DELETE to "DELETE" string.
};

/// @brief Struct representing an HTTP request.
/// @param method The method of the HTTP request (e.g., GET, POST).
/// @param url The parsed URL of type `ParsedUrl`.
/// @param headers A map of headers to be sent with the request.
/// @param data The data payload to be sent with the request (for POST/PUT).
/// @param outputFile The file path to which the response body will be written if provided.
struct HttpRequest {
    HttpMethod method;                        ///< HTTP method to use.
    ParsedUrl url;                            ///< Parsed URL of the request.
    std::map<std::string, std::string> headers; ///< Headers to be included in the request.
    std::string data;                         ///< Data to be sent in the request body.
    std::string outputFile;                   ///< Optional file path to save the response body.
};

/// @brief Struct representing an HTTP response.
/// @param statusLine The status line of the HTTP response (e.g., "HTTP/1.1 200 OK").
/// @param headers The headers received in the HTTP response.
/// @param body The body content of the HTTP response.
struct HttpResponse {
    std::string statusLine;                    ///< The status line of the response.
    std::vector<std::string> headers;          ///< List of headers received in the response.
    std::string body;                          ///< Body of the response.
};

/// @brief Class to handle HTTP requests, including SSL connections and file downloads.
class HttpClient {
public:
    /// @brief Constructor for HttpClient, initializes SSL context.
    HttpClient();

    /// @brief Destructor for HttpClient, cleans up SSL resources.
    ~HttpClient();

    static std::string generateRequest(const HttpRequest& request);

    /// @brief Sends the HTTP request to the server and receives the response.
    /// @param request The HTTP request to send, containing relevant information.
    /// @param verbose Flag to enable verbose output of the request and response details.
    /// @return The HTTP response received from the server.
    /// @throws std::runtime_error if any error occurs during the request.
    HttpResponse sendRequest(const HttpRequest& request, bool verbose);

    /// @brief Downloads a file from the server using the specified HTTP request.
    /// @param request The HTTP request containing the file URL and download details.
    /// @param verbose Flag to enable verbose output of the download process.
    void downloadFile(const HttpRequest& request, bool verbose);

private:
    SSL_CTX* ctx; ///< SSL context used for establishing secure connections.

    /// @brief Initializes SSL and sets up the SSL context.
    void initSSL();

    /// @brief Cleans up SSL resources and frees the SSL context.
    void cleanSSL();

    /// @brief Creates an SSL connection over an existing socket.
    /// @param socket The socket file descriptor connected to the server.
    /// @param hostname The hostname of the server for SSL validation.
    /// @return A pointer to an SSL object representing the secure connection.
    SSL* createSSLConnection(int socket, const std::string& hostname);

    /// @brief Verifies the server's SSL certificate against the provided hostname.
    /// @param ssl The SSL object representing the established connection.
    /// @param hostname The server's hostname for verification purposes.
    /// @return True if the certificate is verified successfully, false otherwise.
    bool verifySSLCert(SSL* ssl, const std::string& hostname);

    /// @brief Creates a socket and connects it to the specified server and port.
    /// @param hostname The server's hostname.
    /// @param port The port to connect to (usually 80 for HTTP or 443 for HTTPS).
    /// @return The socket file descriptor if the connection is successful.
    /// @throws std::runtime_error if the connection fails.
    static int createSocket(const std::string& hostname, uint16_t port);

    /// @brief Handles SSL errors during secure communication.
    /// @param ssl The SSL object representing the secure connection.
    /// @param result The result code from the SSL operation.
    /// @throws std::runtime_error if an SSL error occurs.
    static void handleSSLError(SSL* ssl, int result);
};
