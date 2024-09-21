//http_client.cpp

#include "http_client.h"
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <sstream>



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


/// @brief Sends an HTTP request and receives the response from the server.
/// @param parsedUrl The parsed URL struct containing the host and port.
/// @param request The generated HTTP request string.
/// @param verbose Flag indicating if verbose output should be printed.
/// @return The HttpResponse struct containing the status, headers, and body of the response.
/// @throws runtime_error if socket creation, connection, or data transmission fails.
HttpResponse HttpClient::sendRequest(const HttpRequest& request, bool verbose) {
    std::string requestStr = generateRequest(request);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1) {
        throw std::runtime_error(
            "Failed to create socket! "
        );
    }
    
    struct hostent *host = gethostbyname(request.url.host.c_str());
    if(host == NULL) {
        close(sock);
        throw std::runtime_error(
            "Failed to resolve hostname"
        );
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(request.url.port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);


    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        throw std::runtime_error(
            "Connection failed"
        );
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

    if (send(sock, requestStr.c_str(),requestStr.length(),0) < 0) {
        close(sock);
        throw std::runtime_error(
            "Failed to send Request!"
        );
    }
    
    HttpResponse response;
    char buffer[4096];
    int bytes_recieved;
    bool headers_done = false;
    std::string raw_response;
    
    while((bytes_recieved = recv(sock,buffer, sizeof(buffer),0)) > 0) {
        raw_response.append(buffer,bytes_recieved);
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

