#pragma once

#include "node.hpp"
#include <mutex>

#include <iostream>
// Bootnode for initial node connection
class BootNode : public Node {
    private:
        std::vector<nInf> registry;
        std::mutex registryMutex;

        void registerNode(int connfd){
            nInf incoming;
            if(recvNode(connfd, incoming) < 0){
                perror("Boot failed to receive node");
                return;
            }

            //std::vector<neighbor> 
            {
                std::lock_guard<std::mutex> lock(registryMutex);
                registry.push_back(incoming);
            }
        }

        void handleConnection(int connfd) override {
            registerNode(connfd);
            std::cout << registry.size() << std::endl;
        }

        uint32_t calculateWeight(uint64_t ifBps, uint64_t rfBps = 100000000ULL){
            if (ifBps == 0)return 65535;
            uint64_t weight = rfBps/ifBps;
            if (weight  < 1) return 1;
            if (weight > 65535) return 65535;
            return static_cast<uint32_t>(weight);
        }

    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};