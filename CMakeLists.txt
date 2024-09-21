# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(URLParserProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable with the main source files
add_executable(lurc
    main.cpp
    url_parser.cpp
    http_client.cpp
)

# Include directories
target_include_directories(lurc PUBLIC "${PROJECT_SOURCE_DIR}")

# Optional: Add compiler options (e.g., warnings)
target_compile_options(lurc PRIVATE -Wall -Wextra -pedantic)


# Find Doxygen package
find_package(Doxygen REQUIRED)

# Set the Doxygen input and output directories
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs)

# Configure a target for generating Doxygen documentation
add_custom_target(doc_doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Doxygen documentation"
    VERBATIM
)
