cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

add_compile_options(-std=c++20 -Wall -Wextra -Wsign-conversion)
add_library(Jahan_Signal_Manager STATIC manager.cpp)

install(TARGETS Jahan_Signal_Manager LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)