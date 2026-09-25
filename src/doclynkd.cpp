#include <iostream>
#include <node.h>

int main(int argc, char* argv[]){
    std::cout << "Running daemon" << std::endl;
    return initNode(argc, argv);
}