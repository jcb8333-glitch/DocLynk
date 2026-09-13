#pragma once

#include "node.hpp"
#include "../net/netstat.hpp"
#include <mutex>
#include <queue>

#include <iostream>
// Bootnode for initial node connection
class BootNode : public Node {
    private:
        NetStat net;

        void handleConnection(int connfd, Packet& packet) override {
            if(packet.type == MsgType::RtReq){
                nInf newSuccessor = findSuccessor(packet.payload.id);
                net.onDiscover(packet.payload);
                Packet res{MsgType::RtRes, packet.packetID, packet.chordID, newSuccessor};
                sendPacket(connfd, res);
            } else {
                Node::handleConnection(connfd, packet);
            }
        }

        void route(){}

    public:
        BootNode(const char* selfAddr) : Node(selfAddr, selfAddr){}
};