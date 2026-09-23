#include <token.h>
#include <cassert>

namespace {
    std::unordered_map<uint256, Token> tokens_;
    std::unordered_map<uint256, address> owners_;
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

    void update(address to, uint256 id){
        auto& token = tokens_.at(id);
        if (token.state == LockState::UNLOCKED){
            token.owner = to;
            owners_.at(id) = to;
        }
    }

    void transfer(address from, address to, uint256 id){
        assert(approve(id, from));
        assert(validAddress(to));
        update(to, id);
    }

}