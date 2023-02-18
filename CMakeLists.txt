cmake_minimum_required(VERSION 3.15)
project(FRAME VERSION 0.0.1)

set(CMAKE_CXX_COMPILER /usr/bin/g++)

string(TOLOWER ${PROJECT_NAME} exec_name)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(lib)
add_subdirectory(lib)
add_subdirectory(src)

file(COPY certs/certificate.bundle DESTINATION ${CMAKE_BINARY_DIR})
file(COPY certs/private.key DESTINATION ${CMAKE_BINARY_DIR})