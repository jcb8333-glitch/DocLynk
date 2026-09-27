#include <vault.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    seed(seedGen()),
    addr(sha1Trunc(seed)),
    publicKey(std::move(keys.publicKey)),
    privateKey(std::move(keys.privateKey))
{}