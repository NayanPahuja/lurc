
---

# LURC 

## Overview

This project is an attempt to make a command-line client (like cURL) built in C++ that allows users to perform basic HTTP requests (GET, POST, PUT, DELETE) to a specified URL. The client provides a way to specify HTTP methods, headers, and body data via command-line arguments, making it a simple yet powerful tool for interacting with web servers. The application includes components for URL parsing, request generation, and sending/receiving data over a network.

## Features

## BRANCH_INFO
- This branch is currently working on providing https support using OpenSSL.

- Supports HTTP methods: GET, POST, PUT, DELETE.
- Allows setting custom headers and request data.
- Verbose mode to display request and response details.
- Validates URL formats and ports.
- Sends requests and handles responses using raw socket programming.

## Getting Started

### Prerequisites

- A C++ compiler supporting C++11 or later.
- CMake (recommended for building the project).

### Building the Project

1. Clone the repository:

   ```bash
   git clone https://github.com/NayanPahuja/lurc.git
   cd lurc
   ```

2. Create a build directory and compile:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Run the executable:

   ```bash
   ./lurc
   ```

### Usage

To run the HTTP client, use the following command format:

```bash
./lurc [-v] [-X <method>] [-H <header>] [-d <data>] [-o <file_name>]<URL>
```

#### Command-Line Options

- `-v`: Verbose mode, displays detailed request and response information.
- `-X <method>`: Specifies the HTTP method to use (`GET`, `POST`, `PUT`, `DELETE`). Defaults to `GET`.
- `-H <header>`: Adds a header to the request in the format `Key: Value`.
- `-d <data>`: Specifies the data to send in the body of the request (only applicable for methods like `POST` or `PUT`).
- `<URL>`: The URL to which the request is sent.

#### Example Commands

1. **Basic GET request**:

   ```bash
   ./lurc http://eu.httpbin.org/get
   ```

2. **POST request with headers and data**:

   ```bash
   ./lurc -X POST -H "Content-Type: application/json" -d '{"name": "test"}' http://eu.httpbin.org/post
   ```

3. **Verbose GET request**:

   ```bash
   ./lurc -v http://eu.httpbin.org/get
   ```

4. **Downloading an Image**:
    ```bash
    ./lurc -o test.jpg http://www.keycdn.com/img/example.jpg
    ```

### Error Handling

- If an invalid URL or port is provided, the program will display an appropriate error message.
- Incorrect usage of options will result in detailed error messages guiding the user to correct the input format.

## Project Structure

- **main.cpp**: Handles command-line parsing and controls the flow of the HTTP request process.
- **url_parser.h / url_parser.cpp**: Contains logic to parse URLs into components (protocol, host, port, path).
- **http_client.h / http_client.cpp**: Manages the creation and sending of HTTP requests and the handling of 
responses.

## Contributing

Contributions are welcome!

## Roadmap
- Add HEAD and PATCH method in HTTP.
- Add handling of keep alive and using it to send multiple requests over the same TCP connection.
- Add the ability to continue downloads by adding a flag like in curl -.
- Add SSL and support for HTTPS.
- Add functionality depending on status codes such as Redirection etc.

- Any vulnerabilities or improvement suggestions are always welcome!

## License

This project is licensed under the MIT License.

## Acknowledgments

- Basic socket programming concepts are adapted to provide robust error handling.
- [Checkout Beej's guide to Networking for learning about networks](https://beej.us/guide/bgnet/);
- Regex is used for parsing URLs, inspired by common URL formats in web development.

---
