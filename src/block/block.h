#pragma once

#include <string>
#include <cstdint>
#include <merkle.h>

/*
Contains structure of a block to be held by network nodes.
*/

// Block header to hold meta data
struct Header{
    Block* prev;
    std::string timestamp;
    uint32_t nonce;
};

struct Block{
    Header header;
    MerkleNode root;
};