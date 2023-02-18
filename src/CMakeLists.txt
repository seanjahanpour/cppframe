cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)

add_executable(${exec_name} main.cpp)

target_link_libraries(${exec_name} Jahan_Server_SecureWebSocket Jahan_Signal_Manager Jahan_File Jahan_MariaDB)

file(COPY app.conf DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/log)