cmake_minimum_required(VERSION 3.16)
project(doclynk)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(node src/net/node/node.cpp)
target_include_directories(node PRIVATE src src/net/node)
target_link_libraries(node PRIVATE Threads::Threads OpenSSL::Crypto)