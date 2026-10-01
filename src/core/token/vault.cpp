#include <vault.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    seed(std::move(seedGen())),
    addr(std::move(sha1Trunc(seed))),
    masterKey(std::move(""))
{}

int Vault::genKeyLayer(){
    std::string whole = shaNhex(seed, 512);
    entroCode = std::move(whole.substr((whole.length()/2)+1));
    return 0;
}

int Vault::sign(){}

int Vault::accept(){}