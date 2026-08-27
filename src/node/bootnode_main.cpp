#include "bootnode.hpp"

int main(){
    const char* selfAddr = "";
    BootNode boot(selfAddr);
    boot.joinAll();
}