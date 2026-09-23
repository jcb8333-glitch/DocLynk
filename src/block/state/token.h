#pragma once

#include <string>
#include <uint256.h>

/*
    Contains data structures and abstract methods used in token.cpp
*/

// Track specific token status messages through int value
enum TokenLogs{
    Success = 0,
    InvalidAddress = 1,
    InvalidSender = 2,
    InvalidReceiver = 3,
};

// Lock/Unlock for token transfers and important info changes.
// Used in update() definition.
enum LockState{
    UNLOCKED,
    LOCKED
};

// Struct to store an address to a vault
// Will be implemented properly later once Vault is defined
struct address{
    uint64_t addr;
    bool operator==(const address&) const = default;
};

// Stores token metadata
// Used to represent a token on the network
struct Token{
    uint256 id;
    std::string name;
    std::string URI;
    LockState state;
    address owner;
};

// Checks if an address isn't 0 since no valid vault can have an address of 0
// An address of 0 is reserved for other uses not yet defined
bool validAddress(address addr);


namespace TokenRegistry{
    // Creates a new valid token assigned to a vault
    Token mint(address to, uint256 id);

    // Updates token data with new information
    void update(address to, uint256 id);

    // Changes the address for a token to another effectivly changing ownership
    int transfer(address from, address to, uint256 id);

    // Check if token with id belongs to the proper address
    // Reduces chance of error as network block state changes
    bool approve(uint256 id, address auth);
}
