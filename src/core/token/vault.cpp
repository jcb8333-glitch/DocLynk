#include <vault.h>

std::string seedGen(){
    unsigned char buf[8];
    if (RAND_bytes(buf, sizeof(buf)) != 1) {
        return "";
    }
    const char* res = reinterpret_cast<const char*>(buf);
    return std::string(res);
}

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

int Vault::sign(){
    return 0;
}

int Vault::accept(){
    return 0;
}