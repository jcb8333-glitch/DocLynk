#include <vault.h>
#include <HDT.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    seed(std::move(seedGen())),
    addr(std::move(sha1Trunc(seed)))
{}

int Vault::sign(){}

int Vault::accept(){}