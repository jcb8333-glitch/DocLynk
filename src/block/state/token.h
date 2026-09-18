#pragma once

#include <string>
/*
Contains methods on tokenizing a document and ensuring its validity.
*/
struct MetaData{
    std::string id;
    std::string walletAddr;
    std::string URI;
};

class Token{
    public:
        MetaData meta{};

        Token(){}

        void transfer(std::string dstWallet){
            meta.walletAddr = dstWallet;
        }

    private:
        // Generate ID
};