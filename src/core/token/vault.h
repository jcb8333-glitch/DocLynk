#pragma once

#include <string>
#include <cstdint>
#include <random>
#include <chrono>
#include <HDT.h>
#include <openssl/rand.h>
#include <utils/sha.h>
#include <utils/uint256.h>

/*
Vault contains a hierarchical deterministic tree to hold tokens.
Handles encryption and decryption of 
A vault address is held by a token to know to who the token belongs.
*/
// Used in the vault constructor to generate a uint64 hex string for a seed
std::string seedGen();

class Vault{
    public:
        const uint64_t addr;

        explicit Vault(const char* domainKey);
        
        // On ownership authorized, vault encrypt transaction with public key
        int sign();

        // On token receivec, decrypts token and sets token state to locked 
        // This adds a transatction to mempool
        int accept();

    private:
        const std::string seed;
        const std::array<uint8_t, 32> mPrivKey;
        const std::string mPubKey;
        std::array<uint8_t, 32> entroCode;

        Vault(std::pair<DerivedSet, std::string> seeded);
};