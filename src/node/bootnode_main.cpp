#include <cstring>
#include "bootnode.hpp"
#include <iostream>

int main(int argc, char* argv[]){
    const char* selfAddr = nullptr;
    for(int i = 1; i < argc-1; i++){
        if(std::strcmp(argv[i], "--self") == 0){
            selfAddr = argv[i+1];
        }
    }
    if(selfAddr){
        BootNode boot(selfAddr);
        boot.joinAll();
    } else {
        std::cerr << "Boot address is null" << std::endl;
        return EXIT_FAILURE;
    }
}