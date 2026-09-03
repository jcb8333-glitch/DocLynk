cmake_minimum_required(VERSION 3.16)
project(doclynk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(node src/node/node_main.cpp)
target_link_libraries(node PRIVATE Threads::Threads OpenSSL::Crypto)

add_executable(bootnode src/node/bootnode_main.cpp)
target_link_libraries(bootnode PRIVATE Threads::Threads Openssl::Crypto)
