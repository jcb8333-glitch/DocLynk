#include <vault.h>
#include <HDT.h>

// TODO: Make constructor compatible with const master keys
Vault::Vault():
    seed(std::move(seedGen())),
    addr(std::move(sha1Trunc(seed)))
    
{
    DerivedSet dset = evalMaster(seed); 
    mPrivKey = std::move(dset.privKey);
    entroCode = std::move(dset.chainCode);
}

int Vault::sign(){}

int Vault::accept(){}