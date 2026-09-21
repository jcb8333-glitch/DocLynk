#pragma once

#include <state/token.h>
#include <string>

/*
Contains struct and functions used to create a Merkle root held in a block.
*/

// Represents a node in the Merkle tree
struct MerkleNode {
    MerkleNode* next;
    std::string hash;
};

// Function intended to hash the leaf nodes to build the Merkle root - may change method later
MerkleNode buildRoot(){}

// Helper functions to bould a node from a token or a hash 
MerkleNode buildNode(Token token){}
MerkleNode buildNode(std::string hash1, std::string hash2){}