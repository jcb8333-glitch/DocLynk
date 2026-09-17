#pragma once

#include <string>
#include <cstdint>
#include "merkle.hpp"

struct Block{
    // Header
    // Prev block addr
    // Timestamp
    // Nonce - Proof of work number
    // Merkel root - Holds transactions. Tree?
    Block* prev;
    std::string timestamp;
    uint32_t nonce;
    MerkleNode root;
};