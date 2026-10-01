#pragma once

#include <string>
#include <cstdint>
#include <random>
#include <chrono>
#include <utils/sha.h>
#include <utils/pkeys.h>
#include <utils/uint256.h>

/*
Vault contains a hierarchical deterministic tree to hold tokens.
Handles encryption and decryption of 
A vault address is held by a token to know to who the token belongs.
*/
// Used in the vault constructor to generate a uint64 hex string for a seed
std::string seedGen(){
    const std::string hexchars = "0123456789abcdef";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, hexchars.size() - 1);

    std::string seed;
    seed.reserve(16);
    for(int i = 0; i < 16; ++i){
        seed += hexchars[distribution(generator)];
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();
    seed += std::to_string(nanos);
    return seed;
}   

class Vault{
    public:
        const uint64_t addr;

        Vault()=default;
        
        // On ownership authorized, vault encrypt transaction with public key
        int sign();

        // On token receivec, decrypts token and sets token state to locked 
        // This adds a transatction to mempool
        int accept();

    private:
        const std::string seed;
        const std::string masterKey;
        std::string entroCode;

        Vault(KeyPairPEM keys); 

        int genKeyLayer();
};