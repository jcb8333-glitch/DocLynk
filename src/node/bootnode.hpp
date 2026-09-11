#pragma once

#include "node.hpp"
#include "../net/netstat.hpp"
#include <mutex>
#include <queue>

#include <iostream>
// Bootnode for initial node connection
class BootNode : public Node {
    private:
        std::vector<nInf> registry;
        std::mutex registryMutex;

        void registerNode(int connfd){
            Packet pack;
            if(recvPacket(connfd, pack) < 0){
                perror("Boot failed to receive node");
                return;
            }

            {
                std::lock_guard<std::mutex> lock(registryMutex);
                registry.push_back(pack.payload);
            }
        }

        void handleConnection(int connfd, Packet& packet) override {
            if(packet.type == MsgType::RegReq){
                nInf newSuccessor = findSuccessor(packet.payload.id);
                NetStat::onJoin(packet.payload);
                Packet res{MsgType::RegRes, packet.packetID, packet.chordID, newSuccessor};
                sendPacket(connfd, res);
            } else {
                Node::handleConnection(connfd, packet);
            }
        }

        uint32_t calculateWeight(uint64_t ifBps, uint64_t rfBps = 100000000ULL){
            if (ifBps == 0)return 65535;
            uint64_t weight = rfBps/ifBps;
            if (weight  < 1) return 1;
            if (weight > 65535) return 65535;
            return static_cast<uint32_t>(weight);
        }

        void route(){}

    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};