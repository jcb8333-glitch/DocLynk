#pragma once

#include <string>
#include <uint256.h>
#include <unordered_map>

enum TokenLogs{
    Success = 0,
    InvalidAddress = 1,
    InvalidSender = 2,
    InvalidReceiver = 3,
};

enum LockState{
    UNLOCKED,
    LOCKED
};

struct address{
    uint64_t addr;
    bool operator==(const address&) const = default;
};

struct Token{
    uint256 id;
    std::string name;
    std::string URI;
    LockState state;
    address owner;
};

bool validAddress(address addr);

namespace TokenRegistry{
    Token mint(address to, uint256 id);
    void update(address to, uint256 id);
    int transfer(address from, address to, uint256 id);
    bool approve(uint256 id, address auth);
}
