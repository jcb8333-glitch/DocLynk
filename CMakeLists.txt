cmake_minimum_required(VERSION 3.16)
project(doclynk)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(doclynk
    src/doclynkd.cpp
    src/net/node.cpp
    src/core/token/token.cpp
    src/core/block/block.cpp
)

target_include_directories(doclynk
    PUBLIC  ${CMAKE_SOURCE_DIR}/include # Just in case
    PRIVATE ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/net
            ${CMAKE_SOURCE_DIR}/src/core/token
            ${CMAKE_SOURCE_DIR}/src/core/block
            /usr/local/include  
)

target_link_libraries(doclynk PRIVATE Threads::Threads OpenSSL::SSL)