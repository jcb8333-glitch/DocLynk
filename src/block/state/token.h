#pragma once

#include <string>
#include <uint256.h>
/*
Contains methods on tokenizing a document and ensuring its validity.
*/

// Enum to track token process errors
enum TokenLogs{
    Success = 0,
    InvalidSender = 1,
    InvalidReceiver = 2,
};

// Enum to track current lock state of token
enum LockState{
    UNLOCKED,
    LOCKED
};

// Temporary? struct to hold address type
struct address{
    std::string addr;
};

struct Token{
    const uint256 id;
    std::string name;
    const std::string URI;
    LockState state;
    address owner;
};

namespace TokenRegistry{
    Token Mint();
    // Update values of a token
    void Update(address to, uint256 id, address auth);
    // Transfer ownership of a token from one vault address to another
    void Transfer(address from, address to, uint256 tokenId);
    // Approve if a transaction is possible and should take place
    bool Approve();
    // Returns owner vault address of token
    address OwnerOf(uint256 tokenId);
}
