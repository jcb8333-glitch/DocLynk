#include "node.hpp"

int main(){
    const char* selfAddr = "";
    const char* bootAddr = "";
    Node node(selfAddr, bootAddr);
    node.joinAll();
}