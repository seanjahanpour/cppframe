cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

add_library(simple-websocket-server INTERFACE)
target_include_directories(simple-websocket-server INTERFACE ${SIMPLE_WEBSOCKET_DIR})

find_package(Threads REQUIRED)
target_link_libraries(simple-websocket-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})

find_package(Boost 1.54.0 COMPONENTS system thread coroutine context REQUIRED)
target_link_libraries(simple-websocket-server INTERFACE ${Boost_LIBRARIES})
target_include_directories(simple-websocket-server INTERFACE ${Boost_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)
target_link_libraries(simple-websocket-server INTERFACE ${OPENSSL_LIBRARIES})
target_include_directories(simple-websocket-server INTERFACE ${OPENSSL_INCLUDE_DIR})

add_compile_options(-std=c++20 -Wall -Wextra -Wsign-conversion)
add_library(Jahan_Server_SecureWebSocket STATIC secure_web_socket.cpp wss_endpoint_handler.cpp)
target_link_libraries(Jahan_Server_SecureWebSocket simple-websocket-server)

#install(FILES asio_compatibility.hpp server_ws.hpp client_ws.hpp server_wss.hpp client_wss.hpp crypto.hpp utility.hpp status_code.hpp mutex.hpp DESTINATION include/simple-websocket-server)
install(TARGETS Jahan_Server_SecureWebSocket LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)