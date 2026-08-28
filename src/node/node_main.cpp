#include <cstring>
#include "node.hpp"
#include <iostream>

int main(int argc, char* argv[]){
    const char* selfAddr = nullptr;
    const char* bootAddr = nullptr;

    for(int i = 1; i < argc-1; i++){
        if(std::strcmp(argv[i], "--self") == 0){
            selfAddr = argv[i+1];
        } 
        else if(std::strcmp(argv[i], "--boot") == 0){
            bootAddr = argv[i+1];
        }
    }

    if(selfAddr && bootAddr){
        Node node(selfAddr, bootAddr);
        node.joinAll();
    } else {
        std::cerr << "Node address or boot address is null" << std::endl;
        return EXIT_FAILURE;
    }
}