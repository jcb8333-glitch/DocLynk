#include <cstring>
#include "node.hpp"

int main(int argc, char* argv[]){
    const char* selfAddr = nullptr;
    const char* bootAddr = nullptr;

    for(int i = 1; i < argc-1; i++){
        if(argv[i] == "--self"){
            selfAddr = argv[i+1];
        } 
        else if(argv[i] == "--boot"){
            bootAddr = argv[i+1];
        }
    }

    if(selfAddr && bootAddr){
        Node node(selfAddr, bootAddr);
        node.joinAll();
    } else {
        perror("Node address or boot address is null");
        return EXIT_FAILURE;
    }
}