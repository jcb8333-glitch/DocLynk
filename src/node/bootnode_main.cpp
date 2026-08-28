#include <cstring>
#include "bootnode.hpp"

int main(int argc, char* argv[]){
    const char* selfAddr = nullptr;
    for(int i = 1; i < argc-1; i++){
        if(argv[i] == "--self"){
            selfAddr = argv[i+1];
        }
    }
    if(selfAddr){
        BootNode boot(selfAddr);
        boot.joinAll();
    } else {
        perror("Boot address is null");
        return EXIT_FAILURE;
    }
}