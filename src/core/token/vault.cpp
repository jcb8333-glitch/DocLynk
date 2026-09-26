#include <vault.h>

Vault::Vault() : Vault(generateKeypairPEM()){}

Vault::Vault(KeyPairPEM keys):
    seed(""),
    addr(0),
    publicKey(std::move(keys.publicKey)),
    privateKey(std::move(keys.privateKey))
{}