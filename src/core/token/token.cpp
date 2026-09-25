#include <token.h>
#include <vault.h>
#include <map>

/*
    Includes mappings for token by id and owner by id lookups 
    in addition to function definitions declared in token header
*/

namespace {
    std::map<uint256, Token> tokens_;
    std::map<uint256, address> owners_;
}

bool validAddress(address addr){
    return !(addr.addr == 0);
}

namespace TokenRegistry {

    Token mint(address to, uint256 id){
        Token token{id, "", "", LockState::LOCKED, to};
        owners_.emplace(id, to);
        tokens_.emplace(id, token);
        return token;
    }

    bool approve(uint256 id, address auth){
        return owners_.at(id).addr == auth.addr;
    }

    void update(address to, Token& token){
        if (token.state == LockState::UNLOCKED){
            token.owner = to;
            owners_.at(token.id) = to;
        }
    }

    int transfer(address from, address to, uint256 id){
        assert(approve(id, from));
        if (!(approve(id, from) && validAddress(to))){
            return -1;
        }
        auto& token = tokens_.at(id);
        token.state = LockState::UNLOCKED;
        update(to, token);
        token.state = LockState::LOCKED;
        if (approve(id, to)){
            return 0;
        } else {
            return -2;
        }
    }
}