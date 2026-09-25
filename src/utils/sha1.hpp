#pragma once

#include <openssl/evp.h>
#include <iomanip>

inline std::string sha1Hex(const std::string& in){
    unsigned char digest[EVP_MAX_MD_SIZE];
    uint32_t digestLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
    EVP_DigestUpdate(ctx, in.data(), in.size());
    EVP_DigestFinal_ex(ctx, digest, &digestLen);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (uint32_t i = 0; i < digestLen; ++i){
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return oss.str();
}

inline uint64_t sha1Trunc(const std::string& in){
    unsigned char digest[EVP_MAX_MD_SIZE];
    uint32_t digestLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
    EVP_DigestUpdate(ctx, in.data(), in.size());
    EVP_DigestFinal_ex(ctx, digest, &digestLen);
    EVP_MD_CTX_free(ctx);

    uint64_t id = 0;
    for (int i = 0; i < 8; ++i){
        id =(id << 8) | digest[i];
    }
    return id;
}