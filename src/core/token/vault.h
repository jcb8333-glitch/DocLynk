#pragma once

#include <string>
#include <cstdint>
#include <utils/sha1.h>
#include <utils/pkeys.h>

/*
Vault contains a hierarchical deterministic tree to hold tokens.
Handles encryption and decryption of 
A vault address is held by a token to know to who the token belongs.
*/
class Vault{
    public:
        const uint64_t addr;

        Vault()=default;
        
        // On ownership authorized, vault encrypt transaction with public key
        void sign();

        // On token receivec, decrypts token and sets token state to locked 
        // This adds a transatction to mempool
        void accept();

    private:
        const std::string seed;
        const std::string publicKey;
        const std::string privateKey;

        Vault(KeyPairPEM keys);
};