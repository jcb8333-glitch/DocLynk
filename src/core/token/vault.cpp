#include <vault.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    addr(sha1Trunc(seedGen())),
    publicKey(std::move(keys.publicKey)),
    privateKey(std::move(keys.privateKey))
{}