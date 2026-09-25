#pragma once

#include <string>

/*
Vault contains a hierarchical deterministic tree to hold tokens.
A vault address is held by a token to know to who the token belongs.
*/
class Vault{
    private:
        std::string seed;
};