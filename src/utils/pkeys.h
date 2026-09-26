#pragma once

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <string>
#include <cassert>
#include <stdexcept>

struct KeyPairPEM {
    std::string privateKey;
    std::string publicKey;
};

EVP_PKEY* generateKeypair(){
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if(!ctx) return nullptr;

    if(EVP_PKEY_keygen_init(ctx) <= 0){
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    if(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1) <= 0){
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    EVP_PKEY* pkey = nullptr;
    if(EVP_PKEY_keygen(ctx, &pkey) <= 0){
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

std::string getPrivateKeyPEM(EVP_PKEY* pkey){
    BIO* bio = BIO_new(BIO_s_mem());
    if(!bio) return "";

    if(!PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr)){
        BIO_free(bio);
        return "";
    }

    char* data = nullptr;
    long len = BIO_get_mem_data(bio, &data);
    std::string pem(data, len);

    BIO_free(bio);
    return pem;
}

std::string getPublicKeyPEM(EVP_PKEY* pkey){
    BIO* bio = BIO_new(BIO_s_mem());
    if(!bio) return "";

    if(!PEM_write_bio_PUBKEY(bio, pkey)){
        BIO_free(bio);
        return "";
    }

    char* data = nullptr;
    long len = BIO_get_mem_data(bio, &data);
    std::string pem(data, len);
    BIO_free(bio);
    return pem;
}

KeyPairPEM generateKeypairPEM(){
    EVP_PKEY* pkey = generateKeypair();
    if(!pkey) throw std::runtime_error("Vault: key generation failed");

    KeyPairPEM result{ getPrivateKeyPEM(pkey), getPublicKeyPEM(pkey) };
    EVP_PKEY_free(pkey);

    if(result.privateKey.empty() || result.publicKey.empty()){
        throw std::runtime_error("Vault: PEM encoding failed");
    }
    return result;
}