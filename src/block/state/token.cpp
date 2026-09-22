#include <token.h>
#include <cassert>

using namespace TokenRegistry;
Token Mint(address to, uint256 id){
    Token token{id, "", "", LockState::LOCKED, to};
    return token;
}

bool approve(uint256 id, address auth){
    return (owners_.at(id)  == auth);
}

void transfer(address from, address to, uint256 id){
    assert(approve(id, from));
    assert(validAddress(to));
    
    approve(id, to);
}