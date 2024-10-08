//http_client.cpp

#include "http_client.h"
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <regex>
#include <string>
#include <openssl/x509_vfy.h>

HttpClient::HttpClient() : ctx(nullptr) {
    initSSL();   
}

HttpClient::~HttpClient() {
    cleanSSL();   
}

void HttpClient::initSSL() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if(!ctx) {
        throw std::runtime_error("Failed to create an SSL context");
    }
    SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,nullptr);
    SSL_CTX_set_verify_depth(ctx,4);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        throw std::runtime_error("Failed to set default verify paths");
    }
}

void HttpClient::cleanSSL() {
    if (ctx) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
}


SSL* HttpClient::createSSLConnection(int socket, const std::string &hostname) {
    SSL* ssl = SSL_new(ctx);
    if(!ssl) {
        throw std::runtime_error("Failed to create SSL structure");
    }

    SSL_set_fd(ssl,socket);
    SSL_set_tlsext_host_name(ssl, hostname.c_str());

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        throw std::runtime_error("Failed to establish SSL connection");
    }

    if (!verifySSLCert(ssl,hostname)){
        SSL_free(ssl);
        throw std::runtime_error("SSL Certificate Verification Failed");
    }

    return ssl;
}

bool HttpClient::verifySSLCert(SSL* ssl, const std::string& hostname) {
    // Retrieve the SSL context and X509 store
    X509* cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        return false;
    }

    // Get the verification parameters
    X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
    
    // Set the hostname for verification
    //This automatically invokes the X509_CHECK_HOST in the previous version
    if (!X509_VERIFY_PARAM_set1_host(param, hostname.c_str(), hostname.length())) {
        X509_free(cert);
        return false;
    }

    // Perform verification
    int result = SSL_get_verify_result(ssl);

    // Clean up certificate
    X509_free(cert);

    // Check if the verification succeeded
    return (result == X509_V_OK);
}


void HttpClient::handleSSLError(SSL *ssl, int result) {
    int error = SSL_get_error(ssl,result);
    switch (error) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // Non-blocking IO, you might want to wait and retry
            break;
        case SSL_ERROR_ZERO_RETURN:
            // Connection closed
            throw std::runtime_error("SSL connection closed");
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
            // Other SSL errors
            throw std::runtime_error("SSL error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        default:
            throw std::runtime_error("Unknown SSL error");
    }
}


int HttpClient::createSocket(const std::string& hostname, uint16_t port) {
    struct hostent *host = gethostbyname(hostname.c_str());

    if (!host) {
        throw std::runtime_error("Failed to resolve the hostname");
    }

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1) {
        throw std::runtime_error(
            "Failed to create socket! "
        );
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);


    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        throw std::runtime_error(
            "Connection failed"
        );
    }
    return sock;
}



/// @brief Generates an HTTP request string based on the parsed URL and method.
/// @param parsedUrl The parsed URL struct containing the host and path.
/// @param method The HTTP method to use (GET, POST, etc.).
/// @return The generated HTTP request string.
std::string HttpClient::generateRequest(const HttpRequest& request) {
    std::string methodStr = HttpMethodToString.at(request.method);
    std::string req = methodStr + " " + request.url.path + " HTTP/1.1\r\n";
    req += "Host: " + request.url.host + "\r\n";
    for (const auto& header : request.headers) {
        req += header.first + ": " + header.second + "\r\n";
    }
    if (!request.data.empty()) {
        req += "Content-Length: " + std::to_string(request.data.length()) + "\r\n";
    }

    req += "\r\n";
    
    if (!request.data.empty()) {
        req += request.data;
    }
    
    return req;
}   



// HttpResponse HttpClient::sendRequest(const HttpRequest& request, bool verbose, bool followRedirects) {
//     int maxRedirects = 5;
//     HttpRequest currentRequest = request;
//     HttpResponse response;

//     while (maxRedirects > 0) {
//         response = sendSingleRequest(currentRequest, verbose);
        
//         // Parse status code
//         std::string statusCode;
//         std::regex pattern(R"(HTTP/\d+\.\d+\s+(\d{3}))");
//         std::smatch match;
//         if (std::regex_search(response.statusLine, match, pattern)) {
//             statusCode = match[1];
//         }

//         if (followRedirects && (statusCode == "301" || statusCode == "302" || statusCode == "303" || statusCode == "307" || statusCode == "308")) {
//             std::string location;
//             for (const auto& header : response.headers) {
//                 if (header.find("Location: ") == 0) {
//                     location = header.substr(10);
//                     break;
//                 }
//             }
//             if (!location.empty()) {
//                 currentRequest.url = UrlParser::parse(location);
                
//                 // Preserve the original method for 307 and 308 redirects
//                 if (statusCode == "301" || statusCode == "302" || statusCode == "303") {
//                     currentRequest.method = HttpMethod::GET;
//                 }
                
//                 // Clear the request body for GET requests
//                 if (currentRequest.method == HttpMethod::GET) {
//                     currentRequest.data.clear();
//                 }

//                 if (verbose) {
//                     std::cout << "Following redirect to: " << location << std::endl;
//                 }
//                 maxRedirects--;
//             } else {
//                 break; // No Location header found, stop redirecting
//             }
//         } else {
//             break; // Not a redirect code
//         }
//     }
//     return response;
// }   
HttpResponse HttpClient::sendRequest(const HttpRequest& request, bool verbose) {
    std::string requestStr = generateRequest(request);
    int sock = createSocket(request.url.host,request.url.port);
    SSL* ssl = nullptr;
    if(request.url.protocol == "https") {
        ssl = this->createSSLConnection(sock,request.url.host);
    }
    if (verbose) {
        std::cout <<"> " << requestStr.substr(0, requestStr.find("\r\n")) << std::endl;
        std::istringstream iss(requestStr);
        std::string line;
        std::getline(iss,line); //skip the first line
        while(std::getline(iss,line) && !line.empty()) {
            std::cout << "> " << line << std::endl;
        }
        std::cout << "> " << std::endl;
    }

    if(ssl) {
        if(SSL_write(ssl,requestStr.c_str(),requestStr.length()) <= 0) {
            this->handleSSLError(ssl,0);
        }
    }
    else {
        if (send(sock, requestStr.c_str(),requestStr.length(),0) < 0) {
        close(sock);
        throw std::runtime_error(
            "Failed to send Request!"
        );
    
        }
    }


    
    HttpResponse response;
    char buffer[4096];
    int bytes_recieved; 
    std::string raw_response;


    while (true)
    {
        if(ssl) {
            bytes_recieved  = SSL_read(ssl,buffer,sizeof(buffer));
            if(bytes_recieved <= 0) {
                int err = SSL_get_error(ssl,bytes_recieved);
                if(err == SSL_ERROR_ZERO_RETURN) {
                    break;
                }
                else {
                    this->handleSSLError(ssl,bytes_recieved);
                }
            }
        }
        else {
            bytes_recieved = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_recieved <= 0) {
                break;
            }
        }
        raw_response.append(buffer, bytes_recieved);
    }
    
    
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(sock);
    //Parse the raw response
    std::istringstream resposne_stream(raw_response);
    std::getline(resposne_stream,response.statusLine);
    if (verbose) std::cout << "< " << response.statusLine << std::endl;
    std::string header;
    while(std::getline(resposne_stream,header) && header != "\r") {
        if(!header.empty() && header != "\r") {
            response.headers.push_back(header);
            if (verbose) std::cout << "< " << header << std::endl;
        }
    }

    if (verbose) std::cout << "<" << std::endl;

    std::string body((std::istreambuf_iterator<char>(resposne_stream)), std::istreambuf_iterator<char>());
    response.body = body;
    return response;
}

void HttpClient::downloadFile(const HttpRequest& request, bool verbose) {
    std::string requestStr = generateRequest(request);
    int sock = createSocket(request.url.host, request.url.port);
    SSL* ssl = nullptr;

    // Create SSL connection if protocol is HTTPS
    if (request.url.protocol == "https") {
        ssl = this->createSSLConnection(sock, request.url.host);
    }

    // Verbose output of the request being sent
    if (verbose) {
        std::cout << "> " << requestStr.substr(0, requestStr.find("\r\n")) << std::endl;
        std::istringstream iss(requestStr);
        std::string line;
        std::getline(iss, line); // Skip the first line
        while (std::getline(iss, line) && !line.empty()) {
            std::cout << "> " << line << std::endl;
        }
        std::cout << "> " << std::endl;
    }

    // Send the request using SSL or plain socket
    if (ssl) {
        if (SSL_write(ssl, requestStr.c_str(), requestStr.length()) <= 0) {
            this->handleSSLError(ssl, 0);
        }
    } else {
        if (send(sock, requestStr.c_str(), requestStr.length(), 0) < 0) {
            close(sock);
            throw std::runtime_error("Failed to send Request!");
        }
    }

    // Open the output file to write the response body
    std::ofstream outFile(request.outputFile, std::ios::binary);
    if (!outFile) {
        close(sock);
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        throw std::runtime_error("Failed to open output file: " + request.outputFile);
    }

    char buffer[4096];
    int bytes_received;
    bool headers_done = false;
    std::string header_buffer;

    // Receive the response
    while (true) {
        if (ssl) {
            bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
            if (bytes_received <= 0) {
                int err = SSL_get_error(ssl, bytes_received);
                if (err == SSL_ERROR_ZERO_RETURN) {
                    break; // SSL connection closed cleanly
                } else {
                    this->handleSSLError(ssl, bytes_received);
                }
            }
        } else {
            bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }
        }

        // Process headers and write the response body to the file
        if (!headers_done) {
            header_buffer.append(buffer, bytes_received);
            size_t header_end = header_buffer.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                headers_done = true;
                if (verbose) {
                    std::istringstream headerStream(header_buffer.substr(0, header_end));
                    std::string line;
                    while (std::getline(headerStream, line)) {
                        std::cout << "< " << line << std::endl;
                    }
                    std::cout << "< " << std::endl;
                }
                // Write body part after headers
                outFile.write(buffer + header_end + 4, bytes_received - (header_end + 4));
            }
        } else {
            // Headers are done, write the rest of the body directly
            outFile.write(buffer, bytes_received);
        }
    }

    // Clean up SSL and socket resources
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(sock);
    outFile.close();

    // Verbose message on successful download
    if (verbose) {
        std::cout << "File downloaded successfully: " << request.outputFile << std::endl;
    }
}