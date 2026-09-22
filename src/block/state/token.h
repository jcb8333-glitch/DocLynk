#pragma once

#include <string>
#include <uint256.h>
#include <unordered_map>
/*
Contains methods on tokenizing a document and ensuring its validity.
*/

std::unordered_map<uint256, address> owners_;

// Enum to track token process errors
enum TokenLogs{
    Success = 0,
    InvalidAddress = 1,
    InvalidSender = 2,
    InvalidReceiver = 3,
};

// Enum to track current lock state of token
enum LockState{
    UNLOCKED,
    LOCKED
};

// Temporary? struct to hold address type
struct address{
    uint64_t addr;
};

struct Token{
    const uint256 id;
    std::string name;
    const std::string URI;
    LockState state;
    address owner;
};

bool validAddress(address addr){
    return !(addr.addr == 0);
}

namespace TokenRegistry{
    Token Mint();
    // Update values of a token
    void update(address to, uint256 id, address auth);
    // Transfer ownership of a token from one vault address to another
    void transfer(address from, address to, uint256 tokenId);
    // Approve if a transaction is possible and should take place
    bool approve();
}
