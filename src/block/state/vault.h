#pragma once

#include <string>
#include <cstdint>
#include <hash/sha1.hpp>

/*
Vault contains a hierarchical deterministic tree to hold tokens.
Handles encryption and decryption of 
A vault address is held by a token to know to who the token belongs.
*/
class Vault{
    public:
        const uint64_t addr;

        Vault();
        
        void sign(){
            // On ownership authorized, vault encrypt transaction with public key
        }
        void accept(){
            // On token receivec, decrypts token and sets token state to locked 
            // This adds a transatction to mempool
        }
    private:
        const std::string seed;
        const int publicKey;
        const int privateKey;
};