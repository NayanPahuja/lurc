# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(lurc VERSION 1.0)

find_package(OpenSSL REQUIRED)
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable with the main source files
add_executable(lurc
    main.cpp
    url_parser.cpp
    http_client.cpp
)


target_link_libraries(lurc OpenSSL::SSL OpenSSL::Crypto)

# Include directories
# Include OpenSSL headers (if necessary)
target_include_directories(lurc PRIVATE ${OPENSSL_INCLUDE_DIR})
target_include_directories(lurc PUBLIC "${PROJECT_SOURCE_DIR}")

# Optional: Add compiler options (e.g., warnings)
target_compile_options(lurc PRIVATE -Wall -Wextra -pedantic)

