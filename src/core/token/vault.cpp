#include <vault.h>

Vault::Vault() : Vault([]{
    std::string s = seedGen();
    DerivedSet d = evalMaster(s);
    return std::make_pair(std::move(d), std::move(s));
}()){}

Vault::Vault(std::pair<DerivedSet, std::string> seeded):
    seed(std::move(seeded.second)),
    addr(std::move(sha1Trunc(seed))),
    mPrivKey(std::move(seeded.first.privKey)),
    entroCode(std::move(seeded.first.chainCode))
    
{}

int Vault::sign(){}

int Vault::accept(){}