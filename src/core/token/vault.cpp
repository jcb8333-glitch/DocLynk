#include <vault.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    seed(std::move(seedGen())),
    addr(std::move(sha1Trunc(seed))),
    masterKey(std::move(""))
{}

int Vault::sign(){}

int Vault::accept(){}