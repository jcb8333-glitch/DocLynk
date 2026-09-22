#include <token.h>
#include <cassert>

using namespace TokenRegistry;
Token mint(address to, uint256 id){
    Token token{id, "", "", LockState::LOCKED, to};
    return token;
}

bool approve(uint256 id, address auth){
    return (owners_.at(id)  == auth.addr);
}

void update(address to, Token token){
    if(token.state == LockState::UNLOCKED){
        token.owner = to;
        owners_.at(token.id) = token.owner;
    }
}

void transfer(address from, address to, Token token){
    assert(approve(token.id, from));
    assert(validAddress(to));
    
    approve(token.id, to);
}