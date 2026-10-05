#pragma once

#include <array>
#include <string>
#include <cstring>
#include <cstdint>
#include <openssl/hmac.h>


struct DerivedSet{
    std::array<uint8_t, 32> privKey;
    std::array<uint8_t, 32> chainCode;
};

void setDerivedVals(DerivedSet& dset, const unsigned char* out){
    std::copy(out, out + 32, dset.privKey.begin());
    std::copy(out + 32, out + 64, dset.chainCode.begin());
}

DerivedSet deriveChild(const DerivedSet& parent, uint32_t idx){
    std::array<uint8_t, 36> data{};
    std::copy(parent.privKey.begin(), parent.privKey.end(), data.begin());
    data[32] = (idx >> 24) & 0xFF;
    data[33] = (idx >> 16) & 0xFF;
    data[34] = (idx >> 8) & 0xFF;
    data[35] = idx & 0xFF;

    unsigned char out[64];
    unsigned int outlen = 0;
    HMAC(EVP_sha512(), parent.chainCode.data(), parent.chainCode.size(),
        data.data(), data.size(), out, &outlen);

    DerivedSet child{};
    setDerivedVals(child, out);
    return child;
}

DerivedSet evalMaster(const std::string& seed, const char* domainKey){
    unsigned char out[64];
    uint32_t outlen = 0;
    HMAC(EVP_sha512(), domainKey, std::strlen(domainKey), 
        reinterpret_cast<const unsigned char*>(seed.data()), seed.size(), out, &outlen);
    DerivedSet master{};
    setDerivedVals(master, out);
    return master;
}