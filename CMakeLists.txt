cmake_minimum_required(VERSION 3.16)
project(doclynk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(node src/node_main.cpp)
target_link_libraries(node PRIVATE Threads::Threads)

add_executable(bootnode src/bootnode_main.cpp)
target_link_libraries(bootnode PRIVATE Threads::Threads)