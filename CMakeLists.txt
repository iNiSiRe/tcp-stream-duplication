cmake_minimum_required(VERSION 3.3)
project(tcp_stream_duplicate)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/)

add_executable(tcp_stream_duplicate main.cpp src/TcpStreamDuplicator.cpp src/TcpStreamDuplicator.h src/SocketClient.h)

target_link_libraries(tcp_stream_duplicate pthread)
